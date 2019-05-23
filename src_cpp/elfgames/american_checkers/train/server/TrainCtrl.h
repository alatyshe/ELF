/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

// elf
#include "elf/base/context.h"
#include "elf/base/dispatcher.h"
#include "elf/concurrency/ConcurrentQueue.h"
#include "elf/concurrency/Counter.h"
#include "elf/logging/IndexedLoggerFactory.h"
// game
#include "../data_loader.h"
#include "../control/CtrlEval.h"
#include "../control/CtrlSelfplay.h"
#include "../../common/GameStats.h"
#include "../../common/Notifier.h"

using namespace std::chrono_literals;
using ReplayBuffer = elf::shared::ReaderQueuesT<GameRecord>;
using ThreadedCtrlBase = elf::ThreadedCtrlBase;
using Ctrl = elf::Ctrl;
using Addr = elf::Addr;

/*
  waitForSufficientSelfplay - a method that tells if enough games have 
  been played to update the current model.

  ReplayBuffer - Uses here just for cleaning it if keep_prev_selfplay - false.
  SelfPlaySubCtrl - Contains selfplay Records received from clients.
  EvalSubCtrl - Contains eval Records received from clients.
*/
class ThreadedCtrl : public ThreadedCtrlBase {
 public:
  ThreadedCtrl(
      Ctrl&             ctrl,
      elf::GameClient*  client,
      ReplayBuffer*     replay_buffer,
      const GameOptions&  gameOptions,
      const elf::ai::tree_search::TSOptions& mcts_opt)
      : ThreadedCtrlBase(ctrl, 10000),
        replay_buffer_(replay_buffer),
        gameOptions_(gameOptions),
        client_(client),
        rng_(time(NULL)),
        logger_(elf::logging::getIndexedLogger(
              MAGENTA_B + std::string("|++|") + COLOR_END + 
              "ThreadedCtrl-", 
              "")) {
    selfplaySubCtrl_.reset(new SelfPlaySubCtrl(gameOptions_, mcts_opt));
    evalSubCtrl_.reset(new EvalSubCtrl(gameOptions_, mcts_opt));

    ctrl_.reg();
    ctrl_.addMailbox<_ModelUpdateStatus>();
  }

  void Start() {
    if (!ctrl_.isRegistered()) {
      ctrl_.reg();
    }
    ctrl_.addMailbox<_ModelUpdateStatus>();
    start<std::pair<Addr, int64_t>>();
  }

  void waitForSufficientSelfplay(int64_t selfplay_ver) {
    SelfPlaySubCtrl::CtrlResult res;
    // Checks for batch filling for our model every 60 seconds.
    while ((res = selfplaySubCtrl_->needWaitForMoreSample(selfplay_ver)) ==
           SelfPlaySubCtrl::CtrlResult::INSUFFICIENT_SAMPLE) {
      logger_->info(
          "{}Insufficient sample{} for model {}... waiting 60s",
          WHITE_B,
          COLOR_END,
          selfplay_ver);
      logger_->info("{}", replay_buffer_->info());

      std::this_thread::sleep_for(60s);
    }

    if (res == SelfPlaySubCtrl::CtrlResult::SUFFICIENT_SAMPLE) {
      logger_->info(
          "{}Sufficient{} sample for model {}", 
          GREEN_B, 
          COLOR_END, 
          selfplay_ver);
      logger_->info("{}", replay_buffer_->info());
      
      selfplaySubCtrl_->notifyCurrentWeightUpdate();
    }
  }

  void updateModel(int64_t new_model) {
    sendToThread(std::make_pair(ctrl_.getAddr(), new_model));
    _ModelUpdateStatus dummy;
    ctrl_.waitMail(&dummy);
  }

  bool checkNewModel(ClientManager* manager) {
    int64_t new_model = evalSubCtrl_->updateState(*manager);

    // If there is at least one true eval.
    if (new_model >= 0) {
      updateModel(new_model);
      return true;
    }

    return false;
  }

  bool setInitialVersion(int64_t init_version) {
    logger_->info("Setting init version: {}", init_version);
    evalSubCtrl_->setBaselineModel(init_version);

    if (selfplaySubCtrl_->getCurrModel() < 0) {
      selfplaySubCtrl_->setCurrModel(evalSubCtrl_->getBestModel());
    }
    return true;
  }

  /* 
    Adds a new model for evaluation. 
    If --eval_num_games set to 0, then updates the previous model.
  */
  void addNewModelForEvaluation(int64_t selfplay_ver, int64_t new_version) {
    if (gameOptions_.eval_num_games == 0) {
      logger_->info("Update old model without evaluation; eval_num_games={}", gameOptions_.eval_num_games);

      // And send a message to start the process.
      updateModel(new_version);
    } else {
      // logger_->info("Add model for evaluation; selfplay_ver={}, new_version={}, eval_num_games={}", 
      //  selfplay_ver,
      //  new_version,
      //  gameOptions_.eval_num_games);
      // eval_ - std::unique_ptr<EvalSubCtrl>
      evalSubCtrl_->addNewModelForEvaluation(selfplay_ver, new_version);
      /*
        We expect the next batch from the client. 
        If it is offline_train, it means that we train on the data 
        that we have already prepared.
        In the case of game, we are waiting for the next completed 
        batch from client for training on it.
      */
      if (gameOptions_.mode != "offline_train") {
        waitForSufficientSelfplay(selfplay_ver);
      }
    }
  }

  void setEvalMode(int64_t new_ver, int64_t old_ver) {
    logger_->info("setEvalMode old_ver:{}, new_ver:{}\n", old_ver, new_ver);

    evalSubCtrl_->setBaselineModel(old_ver);
    evalSubCtrl_->addNewModelForEvaluation(old_ver, new_ver);
    eval_mode_ = true;
  }

  // Call by writer thread.
  std::vector<FeedResult> onSelfplayGames(const std::vector<GameRecord>& records) {
    // Receive selfplay/evaluation games.
    std::vector<FeedResult> res(records.size());

    // selfPlaySubCtrl -> SelfPlaySubCtrl
    for (size_t i = 0; i < records.size(); ++i) {
      res[i] = selfplaySubCtrl_->feed(records[i]);
    }
    // std::cout << "onSelfplayGames len : " << res.size() << std::endl;
    return res;
  }

  std::vector<FeedResult> onEvalGames(
      const ClientInfo& info,
      const std::vector<GameRecord>& records) {
    // Receive selfplay/evaluation games.
    std::vector<FeedResult> res(records.size());

    for (size_t i = 0; i < records.size(); ++i) {
      res[i] = evalSubCtrl_->feedStats(info, records[i]);
    }
    // std::cout << "onEvalGames len : " << res.size() << std::endl;
    return res;
  }

  void fillInRequest(const ClientInfo& info, MsgRequest* request) {
    request->vers.set_wait();
    request->client_ctrl.client_type = info.type();

    switch (info.type()) {
      case CLIENT_SELFPLAY_ONLY:
        if (!eval_mode_) {
          selfplaySubCtrl_->fillInRequest(info, request);
        }
        break;
      case CLIENT_EVAL_THEN_SELFPLAY:
        evalSubCtrl_->fillInRequest(info, request);
        if (request->vers.wait() && !eval_mode_) {
          selfplaySubCtrl_->fillInRequest(info, request);
        }
        break;
      case CLIENT_INVALID:
        logger_->info("Warning! Invalid client_type! ");
        break;
    }
  }

 protected:
  enum _ModelUpdateStatus { MODEL_UPDATED };

  ReplayBuffer*   replay_buffer_ = nullptr;

  std::unique_ptr<SelfPlaySubCtrl>  selfplaySubCtrl_;
  std::unique_ptr<EvalSubCtrl>      evalSubCtrl_;

  bool eval_mode_ = false;

  const GameOptions gameOptions_;
  elf::GameClient* client_ = nullptr;
  std::mt19937 rng_;

 private:
  std::shared_ptr<spdlog::logger> logger_;

  /* 
    Reports python for a new, better model.
  */
  void on_thread() override {
    std::pair<Addr, int64_t> data;
    // ctrl_ -> CtrlT<Queue>;
    if (!ctrl_.peekMail(&data, 0)){
      // std::cout << "num : " << data.second << std::endl;
      // std::cout << "id  : " << data.first.id << std::endl;
      // std::cout << "label : " << data.first.label << std::endl;
      return;
    }

    // std::cout << "num : " << data.second << std::endl;
    // std::cout << "id  : " << data.first.id << std::endl;
    // std::cout << "label : " << data.first.label << std::endl;
    int64_t ver = data.second;

    evalSubCtrl_->setBaselineModel(ver);
    int64_t old_ver = selfplaySubCtrl_->getCurrModel();
    selfplaySubCtrl_->setCurrModel(ver);

    // After setCurrModel, new model from python side with the old selfplay_ver
    // will not enter the replay buffer
    logger_->info("Updating .. old_ver: {}, new_ver: {}", old_ver, ver);
    // A better model is found, clean up old games (or not?)
    if (!gameOptions_.keep_prev_selfplay) {
      replay_buffer_->clear();
    }

    // Data now prepared ready,
    // Send message to deblock the caller.
    ctrl_.sendMail(data.first, MODEL_UPDATED);

    // Then notify the python side for the new selfplay version.
    // Then we send information to Python side.
    MsgVersion msg;
    msg.model_ver = ver;
    elf::FuncsWithState funcs =
        client_->BindStateToFunctions({"train_ctrl"}, &msg);
    client_->sendWait({"train_ctrl"}, &funcs);
  }
};





/* 
  server side
  ReplayBuffer - contains Records of the games that clients played.
    The new model will be trained from this buffer.
*/
class TrainCtrl : public DataInterface {
 public:
  TrainCtrl(
      Ctrl&               ctrl,
      int                 num_games,
      elf::GameClient*    client,
      const GameOptions&  gameOptions,
      const elf::ai::tree_search::TSOptions& mcts_opt)
      : ctrl_(ctrl),
        rng_(time(NULL)),
        recordBufferSimple_(gameOptions.records_buffer_directory + "tc_selfplay"),
        logger_(elf::logging::getIndexedLogger(
              MAGENTA_B + std::string("|++|") + COLOR_END + 
              "TrainCtrl-", 
              "")) {

    // Register sender for python thread.
    elf::shared::RQCtrl rq_ctrl;
    rq_ctrl.num_reader = gameOptions.num_reader;
    rq_ctrl.ctrl.queue_min_size = gameOptions.q_min_size;
    rq_ctrl.ctrl.queue_max_size = gameOptions.q_max_size;

    replay_buffer_.reset(new ReplayBuffer(rq_ctrl));
    logger_->info(
        "Finished initializing replay_buffer(ReplayBuffer). info :\n{}", replay_buffer_->info());
    
    threaded_ctrl_.reset(new ThreadedCtrl(
        ctrl_, client, replay_buffer_.get(), gameOptions, mcts_opt));
    logger_->info(
        "Finished initializing threaded_ctrl_(ThreadedCtrl)");

    client_mgr_.reset(new ClientManager(
        num_games,
        gameOptions.client_max_delay_sec,
        gameOptions.expected_num_clients,
        0));
    logger_->info(
        "Finished initializing client_mgr_(ClientManager). info:\n{}", client_mgr_->info());

  }

  void OnStart() override {
    // Call by shared_rw thread or any thread that will call OnReceive.
    ctrl_.reg("train_ctrl");
    ctrl_.addMailbox<int>();
    threaded_ctrl_->Start();
  }

  ReplayBuffer* getReplayBuffer() {
    return replay_buffer_.get();
  }

  ThreadedCtrl* getThreadedCtrl() {
    return threaded_ctrl_.get();
  }

  bool setEvalMode(int64_t new_ver, int64_t old_ver) {
    logger_->info("Setting eval mode: new: {}, old: {}", new_ver, old_ver);
    client_mgr_->setSelfplayOnlyRatio(0.0);
    threaded_ctrl_->setEvalMode(new_ver, old_ver);
    return true;
  }

  /*
    Method for processing received messages(batches) from client.
    init in DataOnlineLoader::start()
    call from Reader::threaded_receive_msg()
  */
  elf::shared::InsertInfo OnReceive(const std::string&, const std::string& s)
      override {
    GameRecords rs = GameRecords::createFromJsonString(s);

    // rs.identity  - client name from which we got the batch.
    // rs.states    - batch summary(thread_id, seq, move_idx, black/white ver)
    const ClientInfo& info = client_mgr_->updateStates(rs.identity, rs.states);

    // Data comes from a file.
    if (rs.identity.size() == 0) {
      // No identity -> offline data.
      for (auto& r : rs.records) {
        r.offline = true;
      }
    }

    std::vector<FeedResult> selfplay_res =
        threaded_ctrl_->onSelfplayGames(rs.records);

    elf::shared::InsertInfo insert_info;
    for (size_t i = 0; i < rs.records.size(); ++i) {
      if (selfplay_res[i] == FeedResult::FEEDED ||
          selfplay_res[i] == FeedResult::VERSION_MISMATCH) {
        const GameRecord& r = rs.records[i];

        bool black_win = r.result.reward > 0;
        insert_info +=
            replay_buffer_->InsertWithParity(GameRecord(r), &rng_, black_win);
        recordBufferSimple_.feed(r);
        recordBufferSimple_.saveAndClean(1000);
      }
    }
    // threaded_ctrl_ -> std::unique_ptr<ThreadedCtrl>
    std::vector<FeedResult> eval_res =
        threaded_ctrl_->onEvalGames(info, rs.records);

    threaded_ctrl_->checkNewModel(client_mgr_.get());
    recv_count_++;

    if (recv_count_ % 100 == 0) {
      int valid_selfplay = 0, valid_eval = 0;
      for (size_t i = 0; i < rs.records.size(); ++i) {
        if (selfplay_res[i] == FeedResult::FEEDED)
          valid_selfplay++;
        if (eval_res[i] == FeedResult::FEEDED)
          valid_eval++;
      }

      logger_->info(
          "TrainCtrl: Receive data[{}] from {}, #state_update: {}, "
          "#records: {}, #valid_selfplay: {}, #valid_eval: {}",
          recv_count_,
          rs.identity,
          rs.states.size(),
          rs.records.size(),
          valid_selfplay,
          valid_eval);
    }

    return insert_info;
  }

  /*
    Tells the client which model to use 
    which mcts parameters to use 
    and whether the second player needs to be initialized.
  */
  bool OnReply(const std::string& identity, std::string* msg) override {
    ClientInfo& info = client_mgr_->getClient(identity);

    if (info.justAllocated()) {
      logger_->info("New client allocated: {}\n{}", identity, client_mgr_->info());
    }

    MsgRequestSeq request;
    threaded_ctrl_->fillInRequest(info, &request.request);
    request.seq = info.seq();
    *msg = request.dumpJsonString();
    info.incSeq();
    return true;
  }

 private:
  Ctrl& ctrl_;

  std::unique_ptr<ReplayBuffer>   replay_buffer_;
  std::unique_ptr<ClientManager>  client_mgr_;
  std::unique_ptr<ThreadedCtrl>   threaded_ctrl_;

  int recv_count_ = 0;
  std::mt19937 rng_;

  // SelfCtrl has its own record buffer to save EVERY game it has received.
  RecordBufferSimple recordBufferSimple_;

  std::shared_ptr<spdlog::logger> logger_;
};
