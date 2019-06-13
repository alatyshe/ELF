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

// elf
#include "elf/ai/tree_search/tree_search_options.h"
#include "elf/logging/IndexedLoggerFactory.h"

#include "CtrlUtils.h"
#include "../client_manager.h"
#include "../../game/GameOptions.h"


using TSOptions = elf::ai::tree_search::TSOptions;

/*
	Stores Records in RecordBuffer for one model(ver).
	Those records don't use for training!
	Stores simple statistics about Records.
	Has a method that tells whether Records quantities 
	are enough to update the model weights(needWaitForMoreSample).
*/
struct SelfPlayRecord {
 public:
	SelfPlayRecord(int ver, const GameOptions& game_options)
			: ver_(ver),
				game_options_(game_options),
				logger_(elf::logging::getIndexedLogger(
						MAGENTA_B + std::string("|++|") + COLOR_END + 
						"SelfPlayRecord-",
						"")) {
		
    std::string selfplay_prefix = game_options.selfplay_records_directory + 
        "SelfPlayRecord-" + game_options_.server_id + "-" + game_options_.time_signature;
    recordBuffer_.resetPrefix(selfplay_prefix + "-ver_" + std::to_string(ver_));
		
		if (game_options.list_files.size() > 0)
			counter_ = game_options_.selfplay_init_num;

	}

	// Takes new batch and update info, also add Record to our vector.
	void feed(const GameRecord& record) {
		const GameMsgResult& r = record.result;

		const bool didBlackWin = r.reward > 0;
		if (r.draw)
			draw_++;
		else if (didBlackWin) {
			black_win_++;
		} else {
			white_win_++;
		}
		counter_++;

		// recordBuffer_ -> RecordBuffer
		recordBuffer_.feed(record);

		if (r.num_move < 100)
			move0_100++;
		else if (r.num_move < 200)
			move100_200++;
		else if (r.num_move < 300)
			move200_300++;
		else
			move300_up++;

		// shows info every 100 batches
		if (counter_ - last_counter_shown_ >= 500) {
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
			recordBuffer_.saveCurrent();
			recordBuffer_.clear();
			return true;
		} else {
			return false;
		}
	}

	/*
		counter - count of completed games for the model ver_. 
		game_options_.selfplay_init_num - the number of batches to get 
			before upgrading the model for the first time.
		game_options_.selfplay_update_num - 
		num_weight_update_ - 
	*/
	bool needWaitForMoreSample() const {
		logger_->info("Need: {} for model: {}; Counter: {}; Selfplay_init_num: {}; Selfplay_update_num: {}; Num_weight_update: {}; ",
			game_options_.selfplay_init_num +
				game_options_.selfplay_update_num * num_weight_update_,
			ver_,
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
		const int total = black_win_ + white_win_ + draw_;
		const float black_win_rate = static_cast<float>(black_win_) / (black_win_ + white_win_ + 1e-10);

		std::stringstream ss;
		ss  << "==== Record Stats (Model:" << ver_ << ") ====" << std::endl;
		
		ss  << "B_win/W_win : " 
				<< black_win_ 
				<< "/" << white_win_ << std::endl
				<< "Draw :\t" << draw_ << std::endl
				<< "Total :\t" << total << std::endl 
				<< "B winrate = " << black_win_rate * 100 << "%."
				<< std::endl;

		ss  << "Game finished in N moves: " << std::endl
				<< "[  0, 100) = " << move0_100 << std::endl
				<< "[100, 200) = " << move100_200 << std::endl
				<< "[200, 300) = " << move200_300 << std::endl
				<< "[300,  up) = " << move300_up << std::endl;

		ss << "====== End Record Stats =======" << std::endl;

		return ss.str();
	}

 private:
	// statistics.
	const int64_t ver_;
	const GameOptions& game_options_;

	RecordBuffer recordBuffer_;

	int black_win_ = 0, white_win_ = 0, draw_ = 0;
	int move0_100 = 0, move100_200 = 0, move200_300 = 0, move300_up = 0;

	int counter_ = 0;
	int last_counter_shown_ = 0;

	int num_weight_update_ = 0;

	std::shared_ptr<spdlog::logger> logger_;
};


/*
	Contains Records for all models. Those records don't use for training!

	The logic is simple: 
		We get a GameRecord, extract a version of the model from this
		(also checking type of this record), then perform a SelfPlayRecord 
		search in perfs_ for a specific model and add a 
		GameRecord to SelfPlayRecord.
*/
class SelfPlaySubCtrl {
 public:
	enum CtrlResult {
		VERSION_OLD,
		VERSION_INVALID,
		INSUFFICIENT_SAMPLE,
		SUFFICIENT_SAMPLE
	};

	SelfPlaySubCtrl(const GameOptions& game_options, const TSOptions& mcts_options)
			: game_options_(game_options),
				mcts_options_(mcts_options),
				curr_ver_(-1),
				logger_(elf::logging::getIndexedLogger(
						MAGENTA_B + std::string("|++|") + COLOR_END + 
						"SelfPlaySubCtrl-",
						"")) {
	}

	FeedResult feed(const GameRecord& r) {
		std::lock_guard<std::mutex> lock(mutex_);
		// checks for second player(white)
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
			logger_->info("Total SelfPlayRecords: {}, SelfPlay: {} -> {}", 
					perfs_.size() + 1, 
					curr_ver_, 
					ver);
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

	GameOptions			game_options_;
	TSOptions				mcts_options_;
	int64_t					curr_ver_;
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
