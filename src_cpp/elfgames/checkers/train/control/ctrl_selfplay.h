/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <assert.h>
#include <algorithm>
#include <deque>

// ELF
#include "elf/ai/tree_search/tree_search_options.h"
#include "elf/logging/IndexedLoggerFactory.h"

// Checkers
#include "ctrl_utils.h"
#include "../client_manager.h"
#include "../../checkers/CheckersGameOptions.h"


using TSOptions = elf::ai::tree_search::TSOptions;


// Класс отвечающий за сбор инфы от клиента про количество отыгранных игр
// и прочей мелкой статистики.
// + метода который сообщает серверу может ли он начать обучение
// исходя из полноты батча
struct SelfPlayRecord {
 public:
	SelfPlayRecord(int ver, const CheckersGameOptions& game_options)
			: ver_(ver),
				game_options_(game_options),
				logger_(elf::logging::getIndexedLogger(
						MAGENTA_B + std::string("|++|") + COLOR_END + 
						"SelfPlayRecord-",
						"")) {
		std::string selfplay_prefix =
				"selfplay-" + game_options_.server_id + "-" + game_options_.time_signature;
		records_.resetPrefix(selfplay_prefix + "-" + std::to_string(ver_));
	}

	// Получает новый батч и обновляет инфу 
	void feed(const CheckersRecord& record) {
		const CheckersMsgResult& r = record.result;

		const bool didBlackWin = r.reward > 0;
		if (didBlackWin) {
			black_win_++;
		} else {
			white_win_++;
		}
		counter_++;

		// records_ = RecordBuffer
		records_.feed(record);

		if (r.num_move < 100)
			move0_100++;
		else if (r.num_move < 200)
			move100_200++;
		else if (r.num_move < 300)
			move200_300++;
		else
			move300_up++;

		// отображаем общую инфу по батчам каждые 100 игр
		if (counter_ - last_counter_shown_ >= 100) {
			logger_->info("\n{}", info());
			last_counter_shown_ = counter_;
		}
	}

	int n() const {
		return counter_;
	}

	bool is_check_point() const {
		if (game_options_.selfplay_init_num > 0 && game_options_.selfplay_update_num > 0) {
			return (
					counter_ == game_options_.selfplay_init_num ||
					((counter_ > game_options_.selfplay_init_num) &&
					 (counter_ - game_options_.selfplay_init_num) %
									 game_options_.selfplay_update_num ==
							 0));
		} else {
			// Otherwise just save one every 1000 games.
			return counter_ > 0 && counter_ % 1000 == 0;
		}
	}

	bool checkAndSave() {
		if (is_check_point()) {
			records_.saveCurrent();
			records_.clear();
			return true;
		} else {
			return false;
		}
	}

	// true если надо подождать для больше времени для батча
	bool needWaitForMoreSample() const {
		// UNCOMMENT
		logger_->info("Need: {}; Counter: {}; Selfplay_init_num: {}; Selfplay_update_num: {}; Num_weight_update: {}; ",
			game_options_.selfplay_init_num +
				game_options_.selfplay_update_num * num_weight_update_,
			counter_,
			game_options_.selfplay_init_num,
			game_options_.selfplay_update_num,
			num_weight_update_
			);
		
		if (game_options_.selfplay_init_num <= 0){
			return false;
		}
		if (counter_ < game_options_.selfplay_init_num){
			return true;
		}

		if (game_options_.selfplay_update_num <= 0){
			return false;
		}
		// counter - счетчик игр которые уже сыграл клиент 
		// game_options_.selfplay_init_num - нужно вначале отыграть игр после чего
		//        формируется батч
		// game_options_.selfplay_update_num - после N игр обновляем веса
		// num_weight_update_ - количесвто раз которые мы обновляли веса
		return counter_ < game_options_.selfplay_init_num +
				game_options_.selfplay_update_num * num_weight_update_;
	}

	void notifyWeightUpdate() {
		num_weight_update_++;
	}

	void fillInRequest(const ClientInfo&, MsgRequest* msg) const {
		msg->client_ctrl.async = game_options_.selfplay_async;
	}

	std::string info() const {
		const int n = black_win_ + white_win_;
		const float black_win_rate = static_cast<float>(black_win_) / (n + 1e-10);

		std::stringstream ss;
		ss  << "FIX IT(need more info like draw) === Record Stats (" << ver_ << ") ====" << std::endl;
		
		ss  << "B_win/W_win/total: " << black_win_ << "/" << white_win_ << "/" << n 
				<< std::endl 
				<< "B win rate=" << black_win_rate * 100 << "%."
				<< std::endl;

		ss  << "Game finished in N moves: " << std::endl
				<< "[  0, 100)\t="    << move0_100 << std::endl
				<< "[100, 200)\t="  << move100_200 << std::endl;
				// << "[200, 300)\t="  << move200_300 << std::endl;
				// << "[300,  up)\t="   << move300_up << std::endl;

		ss << "=== End Record Stats ====" << std::endl;

		return ss.str();
	}

 private:
	// statistics.
	const int64_t ver_;
	const CheckersGameOptions& game_options_;

	RecordBuffer records_;

	int black_win_ = 0, white_win_ = 0;
	int move0_100 = 0, move100_200 = 0, move200_300 = 0, move300_up = 0;
	int counter_ = 0;
	int last_counter_shown_ = 0;
	int num_weight_update_ = 0;

	std::shared_ptr<spdlog::logger> logger_;
};




class SelfPlaySubCtrl {
 public:
	enum CtrlResult {
		VERSION_OLD,
		VERSION_INVALID,
		INSUFFICIENT_SAMPLE,
		SUFFICIENT_SAMPLE
	};

	SelfPlaySubCtrl(const CheckersGameOptions& game_options, const TSOptions& mcts_options)
			: game_options_(game_options),
				mcts_options_(mcts_options),
				curr_ver_(-1),
				logger_(elf::logging::getIndexedLogger(
						MAGENTA_B + std::string("|++|") + COLOR_END + 
						"SelfPlaySubCtrl-",
						"")) {
	}

	FeedResult feed(const CheckersRecord& r) {
		std::lock_guard<std::mutex> lock(mutex_);
		// Проверка есть ли второй игрок white(чекаем нашу инфу полученную от клиента)
		// если игрок есть, то это не selfplay
		if (!r.request.vers.is_selfplay())
			return NOT_SELFPLAY;
		if (curr_ver_ != r.request.vers.black_ver)
			return VERSION_MISMATCH;

		SelfPlayRecord* perf = find_or_null(r.request.vers.black_ver);
		if (perf == nullptr)
			return NOT_REQUESTED;

		perf->feed(r);
		total_selfplay_++;
		if (total_selfplay_ % 100 == 0) {
			logger_->info(
					"SelfPlaySubCtrl: # total selfplays processed by feed(): {}, curr_version_model: {}",
					total_selfplay_,
					curr_ver_);
		}
		perf->checkAndSave();
		return FEEDED;
	}

	int64_t getCurrModel() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return curr_ver_;
	}

	bool setCurrModel(int64_t ver) {
		std::lock_guard<std::mutex> lock(mutex_);
		if (ver != curr_ver_) {
			logger_->info("SelfPlay: {} -> {}", curr_ver_, ver);
			curr_ver_ = ver;
			find_or_create(curr_ver_);
			return true;
		}
		return false;
	}

	CtrlResult needWaitForMoreSample(int64_t selfplay_ver) const {
		std::lock_guard<std::mutex> lock(mutex_);

		if (selfplay_ver < curr_ver_)
			return VERSION_OLD;

		const auto* perf = find_or_null(curr_ver_);
		if (perf == nullptr)
			return VERSION_INVALID;
		return perf->needWaitForMoreSample() ? INSUFFICIENT_SAMPLE : SUFFICIENT_SAMPLE;
	}

	void notifyCurrentWeightUpdate() {
		std::lock_guard<std::mutex> lock(mutex_);

		auto* perf = find_or_null(curr_ver_);
		assert(perf != nullptr);
		return perf->notifyWeightUpdate();
	}

	int getNumSelfplayCurrModel() {
		std::lock_guard<std::mutex> lock(mutex_);

		auto* perf = find_or_null(curr_ver_);
		if (perf != nullptr)
			return perf->n();
		else
			return 0;
	}

	void fillInRequest(const ClientInfo& info, MsgRequest* msg) {
		std::lock_guard<std::mutex> lock(mutex_);

		if (curr_ver_ < 0) {
			msg->vers.set_wait();
		} else {
			auto* perf = find_or_null(curr_ver_);
			assert(perf != nullptr);
			msg->vers.black_ver = curr_ver_;
			msg->vers.white_ver = -1;
			msg->vers.mcts_opt = mcts_options_;
			perf->fillInRequest(info, msg);
		}
	}

 private:
	mutable std::mutex mutex_;

	CheckersGameOptions			game_options_;
	TSOptions								mcts_options_;
	int64_t									curr_ver_;
	std::unordered_map<int64_t, std::unique_ptr<SelfPlayRecord>> perfs_;

	int64_t total_selfplay_ = 0;

	std::shared_ptr<spdlog::logger> logger_;

	SelfPlayRecord& find_or_create(int64_t ver) {
		auto it = perfs_.find(ver);

		if (it != perfs_.end()) {
			return *it->second;
		}
		auto* record = new SelfPlayRecord(ver, game_options_);
		perfs_[ver].reset(record);
		return *record;
	}

	SelfPlayRecord* find_or_null(int64_t ver) {
		auto it = perfs_.find(ver);

		if (it == perfs_.end()) {
			logger_->info("The version {} was not sent before!", std::to_string(ver));
			return nullptr;
		}
		return it->second.get();
	}

	const SelfPlayRecord* find_or_null(int64_t ver) const {
		auto it = perfs_.find(ver);

		if (it == perfs_.end()) {
			logger_->info("The version {} was not sent before!", std::to_string(ver));
			return nullptr;
		}
		return it->second.get();
	}
};
