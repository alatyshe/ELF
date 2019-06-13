/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>

// elf
#include "elf/logging/IndexedLoggerFactory.h"

#include "DispatcherCallback.h"
#include "../train/distri_base.h"

using ThreadedCtrlBase = elf::ThreadedCtrlBase;

/* 
  --------------------
  --------------------
  --------------------
*/
class ThreadedWriterCtrl : public ThreadedCtrlBase {
 public:
  ThreadedWriterCtrl(
      Ctrl& ctrl,
      const ContextOptions& contextOptions,
      const GameOptions& game_options)
      : ThreadedCtrlBase(ctrl, 0),
        logger_(elf::logging::getIndexedLogger(
            MAGENTA_B + std::string("|++|") + COLOR_END + 
            "ThreadedWriterCtrl-",
            "")) {
    elf::shared::Options netOptions = getNetOptions(contextOptions, game_options);
    writer_.reset(new elf::shared::Writer(netOptions));
    auto currTimestamp = time(NULL);

    logger_->info(
        "Writer info: {}, send ctrl with timestamp {} ",
        writer_->info(),
        currTimestamp);
    writer_->Ctrl(std::to_string(currTimestamp));

    start<>();
  }

  std::string identity() const {
    return writer_->identity();
  }

 protected:
  std::unique_ptr<elf::shared::Writer> writer_;
  int64_t seq_ = 0;
  uint64_t ts_since_last_sent_ = elf_utils::sec_since_epoch_from_now();
  std::shared_ptr<spdlog::logger> logger_;
  // The maximum time to wait for a response from the server is then reconnected.
  static constexpr uint64_t kMaxSecSinceLastSent = 900;

  void on_thread() {
    std::string smsg;
    uint64_t now = elf_utils::sec_since_epoch_from_now();

    // Will block..
    // We constantly expect to receive a message from the server
    if (!writer_->getReplyNoblock(&smsg)) {
      logger_->info(
          "{}No message{}, seq={}, since_last_sec={}",
          YELLOW_C,
          COLOR_END,
          seq_,
          now - ts_since_last_sent_);

      // 900s = 15min
      if (now - ts_since_last_sent_ < kMaxSecSinceLastSent) {
        // logger_->info("Sleep for 10 sec .. ");
        std::this_thread::sleep_for(std::chrono::seconds(10));
      } else {
        logger_->warn(
            "{}No reply for too long{} ({}>{} sec), resending",
            RED_B,
            COLOR_END,
            now - ts_since_last_sent_,
            kMaxSecSinceLastSent);
        getContentAndSend(seq_, false);
      }
      return;
    }
    logger_->info(
        "In reply func: {}Message got{}. since_last_sec={}, seq={}",
        GREEN_B,
        COLOR_END,
        now - ts_since_last_sent_,
        seq_);

    json j = json::parse(smsg);
    MsgRequestSeq msg = MsgRequestSeq::createFromJson(j);
    ctrl_.sendMail("dispatcher", msg.request);
    getContentAndSend(msg.seq, msg.request.vers.wait());
  }

  void getContentAndSend(int64_t msg_seq, bool iswait) {
    if (msg_seq != seq_) {
      logger_->info(
          "Warning! The sequence number [{}] in the msg is different from {}",
          msg_seq,
          seq_);
    }
    std::pair<int, std::string> content;
    ctrl_.call(content);
    if (iswait) {
      std::this_thread::sleep_for(std::chrono::seconds(30));
    } else {
      if (content.first == 0)
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }
    writer_->Insert(content.second);
    seq_ = msg_seq + 1;
    ts_since_last_sent_ = elf_utils::sec_since_epoch_from_now();
  }
};


/* 
  Stores information about the game states 
  also collects records of all completed games for send records on the server.
*/
struct GameGuardedRecords {
 public:
  GameGuardedRecords(const std::string& identity)
      : gameRecords_(identity),
        logger_(elf::logging::getIndexedLogger(
            MAGENTA_B + std::string("|++|") + COLOR_END + 
            "GameGuardedRecords",
            "")) {
  }

  void feed(const GameStateExt& s) {
    std::lock_guard<std::mutex> lock(mutex_);
    gameRecords_.addRecord(s.dumpRecord());
  }

  // call from GameNotifier::OnStateUpdate
  void updateState(const ThreadState& ts) {
    /*
      Information about the game, saved in ThreadState:
      thread_id   - index
      seq         - Which game we have played
      move_idx    - Which move we have proceeded
      black       - version of model
      white       - version of model
    */
    std::lock_guard<std::mutex> lock(mutex_);

    auto now = elf_utils::sec_since_epoch_from_now();

    gameRecords_.updateState(ts);

    last_states_.push_back(std::make_pair(now, ts));
    if (last_states_.size() > 100) {
      last_states_.pop_front();
    }

    if (now - last_state_vis_time_ > 60) {
      std::unordered_map<int, ThreadState> states;
      std::unordered_map<int, uint64_t> timestamps;

      // last_states_ -> std::deque<std::pair<uint64_t, ThreadState>>
      for (const auto& s : last_states_) {
        // unique index of ThreadState = just key to this ThreadState
        timestamps[s.second.thread_id] = s.first;
        states[s.second.thread_id] = s.second;
      }

      logger_->info(
          "{}UpdateStates{} {}",
          BLUE_C,
          COLOR_END,
          visStates(states, &timestamps));

      last_state_vis_time_ = now;
    }
  }

  size_t size() {
    std::lock_guard<std::mutex> lock(mutex_);
    return gameRecords_.records.size();
  }

  std::string dumpAndClear() {
    // send data to server.
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info(
        "{}DumpAndClear(dump all states to JSON and clean){}, #records: {}, {}",
        YELLOW_B,
        COLOR_END,
        gameRecords_.records.size(),
        visStates(gameRecords_.states));

    std::string s = gameRecords_.dumpJsonString();
    gameRecords_.clear();
    return s;
  }

 private:
  std::mutex mutex_;
  GameRecords gameRecords_;
  std::deque<std::pair<uint64_t, ThreadState>> last_states_;
  uint64_t last_state_vis_time_ = 0;
  std::shared_ptr<spdlog::logger> logger_;

  static std::string visStates(
      const std::unordered_map<int, ThreadState>& states,
      const std::unordered_map<int, uint64_t>* timestamps = nullptr) {
    /*
      Information about the game, saved in ThreadState:
      thread_id   - index
      seq         - Which game we have played
      move_idx    - Which move we have proceeded
      black       - version of model
      white       - version of model
    */
    std::stringstream ss;
    ss << "#states: " << states.size();
    ss << "[";

    auto now = elf_utils::sec_since_epoch_from_now();
    std::vector<int> ordered;
    for (const auto& p : states) {
      ordered.push_back(p.first);
    }
    std::sort(ordered.begin(), ordered.end());

    for (const auto& th_id : ordered) {
      auto it = states.find(th_id);
      assert(it != states.end());

      ss  << "id:" << th_id 
          << ";seq:" << it->second.seq 
          << ";#move:" << it->second.move_idx;
      if (timestamps != nullptr) {
        auto it = timestamps->find(th_id);
        if (it != timestamps->end()) {
          uint64_t td = now - it->second;
          ss << ";timestamp:" << td;
        }
      }
      ss << ", ";
    }
    ss << "]  ";
    ss << elf_utils::get_gap_list(ordered);
    return ss.str();
  }
};


/* 
  Used to call the game_end python function from selfplay.py
  Display statistics about the game from python side
  and contain records of finished games.
*/
class GameNotifier : public GameNotifierBase {
 public:
  GameNotifier(
      Ctrl& ctrl,
      const std::string& identity,
      const GameOptions& game_options,
      elf::GameClient* client)
      : ctrl_(ctrl), 
        guardedRecords_(identity), 
        gameOptions_(game_options), 
        client_(client) {
    using std::placeholders::_1;
    using std::placeholders::_2;

    ctrl.RegCallback<std::pair<int, std::string>>(
        std::bind(&GameNotifier::dump_records, this, _1, _2));
  }

  void OnGameEnd(const GameStateExt& s) override {
    // Add state to records.
    guardedRecords_.feed(s);

    FinishReason reason;
    if (s.state().getPly() >= TOTAL_MAX_MOVE)
      reason = MAX_STEP;
    else if (s.state().board().black_win > 0 && s.state().board().white_win > 0 )
      reason = BOTH_REACHED_BASE;
    else if (s.state().board().black_win == 2)
      reason = BLACK_WIN;
    else
      reason = WHITE_WIN;
  
    game_stats_.feedWinRate(reason, s.state().getFinalValue());

    // Report win rate(so that Python side could know).
    elf::FuncsWithState funcs =
        client_->BindStateToFunctions({"game_end"}, &s);
    client_->sendWait({"game_end"}, &funcs);
  }

  void OnStateUpdate(const ThreadState& state) override {
    // Update current state.
    guardedRecords_.updateState(state);
  }

  GameStats& getGameStats() {
    return game_stats_;
  }

 private:
  Ctrl&               ctrl_;
  GameStats           game_stats_;
  GameGuardedRecords  guardedRecords_;
  const GameOptions   gameOptions_;
  elf::GameClient*    client_ = nullptr;

  bool dump_records(const Addr&, std::pair<int, std::string>& data) {
    data.first = guardedRecords_.size();
    data.second = guardedRecords_.dumpAndClear();
    return true;
  }
};



class DistriClient {
 public:
  DistriClient(
      const ContextOptions& contextOptions,
      const GameOptions& game_options,
      elf::GameClient* client)
      : contextOptions_(contextOptions),
        gameOptions_(game_options),
        logger_(elf::logging::getIndexedLogger(
          MAGENTA_B + std::string("|++|") + COLOR_END + 
          "DistriClient-", 
          "")) {
    // ThreadedDispatcher -> elf::ThreadedDispatcherT<MsgRequest, RestartReply>;

    thereadedDispatcher_.reset(new ThreadedDispatcher(ctrl_, contextOptions.num_games));
    dispatcherCallback_.reset(
        new DispatcherCallback(thereadedDispatcher_.get(), client));

    if (gameOptions_.mode == "selfplay") {
      
      writerCtrl_.reset(
          new ThreadedWriterCtrl(ctrl_, contextOptions, game_options));

      GameNotifier_.reset(
          new GameNotifier(ctrl_, writerCtrl_->identity(), game_options, client));

    } else if (gameOptions_.mode == "play") {
    } else {
      throw std::range_error("game_options.mode not recognized! " + gameOptions_.mode);
    }
    logger_->info(
      "{}DistriClient successfully created{}\n",
      GREEN_B, 
      COLOR_END);
  }

  ~DistriClient() {
    GameNotifier_.reset(nullptr);
    thereadedDispatcher_.reset(nullptr);
    writerCtrl_.reset(nullptr);
  }

  ThreadedDispatcher* getDispatcher() {
    return thereadedDispatcher_.get();
  }

  GameNotifier* getNotifier() {
    return GameNotifier_.get();
  }

  const GameStats& getGameStats() const {
    assert(GameNotifier_ != nullptr);
    return GameNotifier_->getGameStats();
  }

  // Transfers to all streams the models versions for black and white players.
  void setRequest(
      int64_t black_ver,
      int64_t white_ver,
      int numThreads = -1) {
    MsgRequest request;
    request.vers.black_ver = black_ver;
    request.vers.white_ver = white_ver;
    request.vers.mcts_opt = contextOptions_.mcts_options;
    request.client_ctrl.num_game_thread_used = numThreads;
    thereadedDispatcher_->sendToThread(request);
  }

 private:
  Ctrl ctrl_;
  /// ZMQClient
  std::unique_ptr<ThreadedDispatcher>   thereadedDispatcher_;
  std::unique_ptr<ThreadedWriterCtrl>   writerCtrl_;

  std::unique_ptr<DispatcherCallback>   dispatcherCallback_;
  std::unique_ptr<GameNotifier> GameNotifier_;

  const ContextOptions                  contextOptions_;
  const GameOptions                     gameOptions_;

  std::shared_ptr<spdlog::logger>       logger_;
};




