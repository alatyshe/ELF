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

// ELF
#include "elf/base/context.h"
#include "elf/base/dispatcher.h"
#include "elf/concurrency/ConcurrentQueue.h"
#include "elf/concurrency/Counter.h"
#include "elf/logging/IndexedLoggerFactory.h"

// Checkers
#include "../data_loader.h"
#include "../control/ctrl_eval.h"
#include "../control/ctrl_selfplay.h"

#include "../../common/game_stats.h"
#include "../../common/notifier.h"

using namespace std::chrono_literals;
using ReplayBuffer = elf::shared::ReaderQueuesT<CheckersRecord>;
using ThreadedCtrlBase = elf::ThreadedCtrlBase;
using Ctrl = elf::Ctrl;
using Addr = elf::Addr;


// Контроль потоков со стороны сервера
// С помощью этого класса сервер постоянно слушает клиентов
// и ожидает от них получения batch(состояний игры и reward за игру)
// выполняет это - метод waitForSufficientSelfplay
// + он контролит выбор лучшей модели, через передачу питону
// по ключу train_ctrl
class ThreadedCtrl : public ThreadedCtrlBase {
 public:
	ThreadedCtrl(
			Ctrl& 						ctrl,
			elf::GameClient* 			client,
			ReplayBuffer* 				replay_buffer,
			const CheckersGameOptions& 	gameOptions,
			const elf::ai::tree_search::TSOptions& mcts_opt)
			: ThreadedCtrlBase(ctrl, 10000),
				replay_buffer_(replay_buffer),
				gameOptions_(gameOptions),
				client_(client),
				rng_(time(NULL)),
				logger_(elf::logging::getIndexedLogger(
							std::string("\x1b[1;35;40m|++|\x1b[0m") + 
							"ThreadedCtrl-", 
							"")) {
		selfplay_.reset(new SelfPlaySubCtrl(gameOptions_, mcts_opt));
		eval_.reset(new EvalSubCtrl(gameOptions_, mcts_opt));

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
		// каждые 30 секунд проверяем на заполненость batch
		// для нашей модели
		while ((res = selfplay_->needWaitForMoreSample(selfplay_ver)) ==
					 SelfPlaySubCtrl::CtrlResult::INSUFFICIENT_SAMPLE) {
			logger_->info(
					"{}Insufficient sample{} for model {}... waiting 30s",
					WHITE_B,
					COLOR_END,
					selfplay_ver);
			std::this_thread::sleep_for(30s);
		}

		if (res == SelfPlaySubCtrl::CtrlResult::SUFFICIENT_SAMPLE) {
			logger_->info(
					"{}Sufficient{} sample for model {}", 
					GREEN_B, 
					COLOR_END, 
					selfplay_ver);
			selfplay_->notifyCurrentWeightUpdate();
		}
	}

	void updateModel(int64_t new_model) {
		sendToThread(std::make_pair(ctrl_.getAddr(), new_model));
		_ModelUpdateStatus dummy;
		ctrl_.waitMail(&dummy);
	}

	bool checkNewModel(ClientManager* manager) {
		int64_t new_model = eval_->updateState(*manager);

		// If there is at least one true eval.
		if (new_model >= 0) {
			updateModel(new_model);
			return true;
		}

		return false;
	}

	bool setInitialVersion(int64_t init_version) {
		logger_->info("Setting init version: {}", init_version);
		eval_->setBaselineModel(init_version);

		if (selfplay_->getCurrModel() < 0) {
			selfplay_->setCurrModel(eval_->getBestModel());
		}
		return true;
	}

	// Добавляем новую модель для сравнения
	// Если eval_num_games == 0, то просто апдейтим предыдущую
	void addNewModelForEvaluation(int64_t selfplay_ver, int64_t new_version) {
		if (gameOptions_.eval_num_games == 0) {
			logger_->info("Update old model without evaluation; eval_num_games={}", gameOptions_.eval_num_games);

			// And send a message to start the process.
			updateModel(new_version);
		} else {
			// logger_->info("Add model for evaluation; selfplay_ver={}, new_version={}, eval_num_games={}", 
			// 	selfplay_ver,
			// 	new_version,
			// 	gameOptions_.eval_num_games);
			// eval_ - std::unique_ptr<EvalSubCtrl>
			eval_->addNewModelForEvaluation(selfplay_ver, new_version);
			// For offline training, we don't need to wait..
			// ожидаем очередной батч от клиента
			// если это offline_train, то подразумевается, что тренируемся на 
			// данных которые у нас уже заготовлены
			// с лучае с шашками, мы ждем очередной заполненный батч для тренировки на нем
			if (gameOptions_.mode != "offline_train") {
				waitForSufficientSelfplay(selfplay_ver);
			}
		}
	}

	void setEvalMode(int64_t new_ver, int64_t old_ver) {

		logger_->info("setEvalMode old_ver:{}, new_ver:{}\n", old_ver, new_ver);

		eval_->setBaselineModel(old_ver);
		eval_->addNewModelForEvaluation(old_ver, new_ver);
		eval_mode_ = true;
	}

	// Call by writer thread.
	std::vector<FeedResult> onSelfplayGames(const std::vector<CheckersRecord>& records) {
		// Receive selfplay/evaluation games.
		std::vector<FeedResult> res(records.size());

		// selfplay_ = SelfPlaySubCtrl
		for (size_t i = 0; i < records.size(); ++i) {
			res[i] = selfplay_->feed(records[i]);
		}

		// std::cout << "onSelfplayGames len : " << res.size() << std::endl;
		return res;
	}

	std::vector<FeedResult> onEvalGames(
			const ClientInfo& info,
			const std::vector<CheckersRecord>& records) {
		// Receive selfplay/evaluation games.
		std::vector<FeedResult> res(records.size());

		for (size_t i = 0; i < records.size(); ++i) {
			res[i] = eval_->feed(info, records[i]);
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
					selfplay_->fillInRequest(info, request);
				}
				break;
			case CLIENT_EVAL_THEN_SELFPLAY:
				eval_->fillInRequest(info, request);
				if (request->vers.wait() && !eval_mode_) {
					selfplay_->fillInRequest(info, request);
				}
				break;
			case CLIENT_INVALID:
				logger_->info("Warning! Invalid client_type! ");
				break;
		}
	}

 protected:
	enum _ModelUpdateStatus { MODEL_UPDATED };

	ReplayBuffer* 	replay_buffer_ = nullptr;
	std::unique_ptr<SelfPlaySubCtrl> selfplay_;
	std::unique_ptr<EvalSubCtrl> eval_;

	bool eval_mode_ = false;

	const CheckersGameOptions gameOptions_;
	elf::GameClient* client_ = nullptr;
	std::mt19937 rng_;

	std::string kTrainCtrl = "train_ctrl";

 private:
	std::shared_ptr<spdlog::logger> logger_;

	// Сообщает о новой лучшей модели
	void on_thread() override {
		// std::cout << "ThreadedCtrl::on_thread" << std::endl;

		std::pair<Addr, int64_t> data;
		// ctrl_ = CtrlT<Queue>;
		if (!ctrl_.peekMail(&data, 0)){
			// std::cout << "num : " << data.second << std::endl;
			// std::cout << "id  : " << data.first.id << std::endl;
			// std::cout << "label : " << data.first.label << std::endl;
			return;
		}

		// std::cout << "num : " << data.second << std::endl;
		// std::cout << "id  : " << data.first.id << std::endl;
		// std::cout << "label : " << data.first.label << std::endl;

		// ОНО СЮДА ДАЖЕ НЕ ДОХОДИТ
		std::cout << "\x1b[1;32;40mYES YES YES YES YES YES YES YES YES YES YES YES\x1b[0m" << std::endl;

		int64_t ver = data.second;

		eval_->setBaselineModel(ver);
		int64_t old_ver = selfplay_->getCurrModel();
		selfplay_->setCurrModel(ver);

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
				client_->BindStateToFunctions({kTrainCtrl}, &msg);
		client_->sendWait({kTrainCtrl}, &funcs);
	}
};



















// server side
class TrainCtrl : public DataInterface {
 public:
	TrainCtrl(
			Ctrl& 											ctrl,
			int 												num_games,
			elf::GameClient* 						client,
			const CheckersGameOptions& 	gameOptions,
			const elf::ai::tree_search::TSOptions& mcts_opt)
			: ctrl_(ctrl),
				rng_(time(NULL)),
				selfplay_record_("tc_selfplay"),
				logger_(
						elf::logging::getIndexedLogger(
							std::string("\x1b[1;35;40m|++|\x1b[0m") + 
							"TrainCtrl-", 
							"")) {

		// Register sender for python thread.
		elf::shared::RQCtrl rq_ctrl;
		rq_ctrl.num_reader = gameOptions.num_reader;
		rq_ctrl.ctrl.queue_min_size = gameOptions.q_min_size;
		rq_ctrl.ctrl.queue_max_size = gameOptions.q_max_size;

		replay_buffer_.reset(new ReplayBuffer(rq_ctrl));
		logger_->info(
				"Finished initializing replay_buffer(ReplayBuffer) info : {}", replay_buffer_->info());
		
		threaded_ctrl_.reset(new ThreadedCtrl(
				ctrl_, client, replay_buffer_.get(), gameOptions, mcts_opt));
		logger_->info(
				"Finished initializing threaded_ctrl_(ThreadedCtrl)");

		client_mgr_.reset(new ClientManager(
				num_games,
				gameOptions.client_max_delay_sec,
				gameOptions.expected_num_clients,
				0.5));
		logger_->info(
				"Finished initializing client_mgr_(ClientManager)", client_mgr_->info());

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

	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	// метод для обработки полученных сообщений(батчей) от клиента
	// инициализируется в DataOnlineLoader::start()
	// вызывается из Reader::threaded_receive_msg()
	elf::shared::InsertInfo OnReceive(const std::string&, const std::string& s)
			override {

		// std::cout << "s : " << s << std::endl << std::endl << std::endl;
		// Заполняем нашу структуру
		CheckersRecords rs = CheckersRecords::createFromJsonString(s);

		// rs.identity  - имя клиента от которого мы получили батч
		// rs.states 		- краткая инфа по батчу(thread_id, seq, move_idx, black/white ver)
		const ClientInfo& info = client_mgr_->updateStates(rs.identity, rs.states);


		// если непонятно откуда пришла инфа, видимо из файла
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
				const CheckersRecord& r = rs.records[i];

				bool black_win = r.result.reward > 0;
				insert_info +=
						replay_buffer_->InsertWithParity(CheckersRecord(r), &rng_, black_win);
				selfplay_record_.feed(r);
				selfplay_record_.saveAndClean(1000);
			}
		}
		// threaded_ctrl_ = std::unique_ptr<ThreadedCtrl>

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

	bool OnReply(const std::string& identity, std::string* msg) override {
		

		ClientInfo& info = client_mgr_->getClient(identity);

		// отправляет клиенту что ему делать, какую модель использовать
		// какие параметры mcts использовать
		// какого игрока инициализировать и пр.
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
	RecordBufferSimple selfplay_record_;

	std::shared_ptr<spdlog::logger> logger_;
};
