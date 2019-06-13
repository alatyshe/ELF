/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fstream>

// elf
#include "elf/ai/tree_search/tree_search_options.h"
#include "elf/logging/IndexedLoggerFactory.h"
#include "elf/utils/utils.h"

#include "CtrlUtils.h"
#include "../client_manager.h"
#include "../fair_pick.h"

using TSOptions = elf::ai::tree_search::TSOptions;

/*
	Use for compare 2 models
*/
class ModelPerfomance {
 public:
	enum EvalResult {
		EVAL_INVALID,
		EVAL_INCOMPLETE,
		EVAL_BLACK_PASS,
		EVAL_BLACK_NOTPASS
	};

	ModelPerfomance(
			const GameOptions& gameOptions,
			const ClientManager& mgr,
			const ModelPair& p)
			: gameOptions_(gameOptions),
				curr_pair_(p),
				logger_(elf::logging::getIndexedLogger(
							MAGENTA_B + std::string("|++|") + COLOR_END + 
							"ModelPerfomance-", 
							"")) {
		const size_t cushion = 5;

		// getExpectedNumEval() - returns number of clients who work in the eval mode.
		const size_t max_request_per_layer = mgr.getExpectedNumEval() / 2;
		// number of times need to swap players
		const size_t num_request = gameOptions.eval_num_games / 2 + cushion;
		const size_t num_eval_machine_per_layer =
				compute_num_eval_machine(num_request, max_request_per_layer);

		games_.reset(new fair_pick::Pick(num_request, num_eval_machine_per_layer));
		
		swap_games_.reset(
				new fair_pick::Pick(num_request, num_eval_machine_per_layer));

    record_.resetPrefix(gameOptions.eval_records_directory + 
        eval_prefix() + "-" + std::to_string(p.black_ver) + "-" +
        std::to_string(p.white_ver));

		logger_->info("{}ModelPerfomance was created{}", GREEN_B, COLOR_END);
	}

	ModelPerfomance(ModelPerfomance&&) = default;

	// Finished games.
	int n_done() const {
		return games_->win_count().n_done() + swap_games_->win_count().n_done();
	}

	// Number of games won.
	int n_win() const {
		return games_->win_count().n_win() + swap_games_->win_count().n_win();
	}

	float winrate() const {
		const int total_games = n_done();
		const int win_games = n_win();
		return total_games == 0 ? 0.0 : static_cast<float>(win_games) / total_games;
	}

	EvalResult eval_result() const {
		return eval_result_;
	}

	std::string info() const {
		std::stringstream ss;
		ss 	<< curr_pair_.info() << "\n" 
				<< "[finished=" << finished_ << "];\t"
				<< "[Winrate=" << winrate() << "]"
				<< "[Win=" << n_win() << "]"
				<< "[Lost=" << n_done() - n_win() << "]"
				<< "[Total=" << n_done() << "]"
				<< "[Draw=" << draw_ << "]"
				<< "; Requests:"
			 	<< "[sent=" << sent_ << "]"
			 	<< "[recieved=" << recv_ << "];"
			 	<< "\u001b[162;1m" << "\nSwap Games-:\t" << "\u001b[0m" << games_->info() 
			 	<< "\u001b[178;1m" << "\nSwap Games+:\t" << "\u001b[0m" << swap_games_->info();
		return ss.str();
	}

	EvalResult updateState(const ClientManager& mgr) {
		if (finished_)
			return eval_result_;

		games_->checkStuck(mgr);
		swap_games_->checkStuck(mgr);

		eval_result_ = eval_check();

		if (n_done() > 0 && (sent_ % 50 == 0 || recv_ % 50 == 0)) {
			logger_->info("{}Eval Status{}: {}\n", 
				GREEN_C,
				COLOR_END,
				info());
		}

		if (finished_ || eval_result_ == EVAL_INCOMPLETE)
			return eval_result_;

		set_finished();
		return eval_result_;
	}

	/*
		Fills rewards for our model_perfomance and add to records pool.
	*/
	void feedInfo(const ClientInfo& c, const GameRecord& r) {
		// мое
		if (r.result.draw) {
			draw_++;
		}
		else if (r.request.client_ctrl.player_swap) {
			swap_games_->add(c, -r.result.reward);
		} else {
			games_->add(c, r.result.reward);
		}
		record_.feed(r);
		recv_++;
	}

	/*
		Fills in the info for the request, which we send to the 
		client and immediately decide whether to swap the players or not
	*/
	void fillInRequest(const ClientInfo& c, MsgRequest* msg) {
		if (finished_)
			return;

		// decide order by checking the number of games.
		std::pair<fair_pick::Pick*, bool> games[2] = {{games_.get(), false},
																									{swap_games_.get(), true}};

		if (games_->n_reg_to_go() < swap_games_->n_reg_to_go()) {
			swap(games[0], games[1]);
		}

		for (const auto& g : games) {
			fair_pick::RegisterResult res = g.first->reg(c);
			if (fair_pick::release_request(res))
				continue;

			// We only use eval_num_threads threads to run evaluation to make it
			// faster.
			msg->vers = curr_pair_;
			// Now treat player_swap as same as other quantities.
			msg->client_ctrl.player_swap = g.second;
			msg->client_ctrl.num_game_thread_used = gameOptions_.eval_num_threads;
			break;
		}
		// number of requests sent 
		sent_++;

		if (sent_ % 50 == 0) {
			logger_->info(
					"{}Sending{} evaluation request #{}:  {}\n",
					YELLOW_C,
					COLOR_END,
					sent_,
					msg->info()
					);
		}
	}

	bool IsFinished() const {
		return finished_;
	}

	const ModelPair& Pair() const {
		return curr_pair_;
	}

 private:
	const GameOptions&	gameOptions_;
	const ModelPair			curr_pair_;

	// For each machine + game_id, the list of rewards.
	// Note that game_id decides whether we swap the player or not.
	std::unique_ptr<fair_pick::Pick>	games_;
	std::unique_ptr<fair_pick::Pick>	swap_games_;

	int						draw_ = 0;
	// The number of requests sent to the client.
	int						sent_ = 0;
	// The number of received messages from the client.
	int						recv_ = 0;
	bool					finished_ = false;
	RecordBuffer	record_;
	EvalResult		eval_result_ = EVAL_INVALID;

	std::shared_ptr<spdlog::logger> logger_;

	static size_t compute_num_eval_machine(size_t n, size_t max_num_eval) {
		if (max_num_eval == 0)
			return 1;

		// if n = 200, max_num_eval = 150, then min_pass = 2, and return 100
		// if n = 200, max_num_eval = 50, then min_pass = 4, and return 50
		size_t min_pass = (n + max_num_eval - 1) / max_num_eval;

		size_t num_eval = (n + min_pass - 1) / min_pass;
		return std::min(num_eval, max_num_eval);
	}

	std::string eval_prefix() const {
		return "eval-" + gameOptions_.server_id + "-" + gameOptions_.time_signature;
	}

	EvalResult eval_check() const {
		const int half_complete = gameOptions_.eval_num_games / 2;
		const float wr = winrate();

		const auto& report = games_->win_count();
		const auto& swap_report = swap_games_->win_count();

		if (report.n_done() >= half_complete &&
				swap_report.n_done() >= half_complete) {
			return wr >= gameOptions_.eval_thres ? EVAL_BLACK_PASS : EVAL_BLACK_NOTPASS;
		}
		/*
		auto res = report.CheckWinrateBound(half_complete, gameOptions_.eval_thres);
		auto swap_res = swap_report.CheckWinrateBound(half_complete,
		gameOptions_.eval_thres);

		if (res == fair_pick::LOSS && swap_res == fair_pick::LOSS) {
			return EVAL_BLACK_NOTPASS;
		}
		if (res == fair_pick::WIN && swap_res == fair_pick::WIN) {
			return EVAL_BLACK_PASS;
		}
		*/

		return EVAL_INCOMPLETE;
	}

	void set_finished() {
		// Save all games.
		finished_ = true;
		logger_->info(
				"{}Eval Finished{}[pass={}]{}\n Saved to file: {}\n\n",
				GREEN_B,
				COLOR_END,
				(eval_result_ == EVAL_BLACK_PASS),
				info(),
				record_.prefix_save_counter());

		record_.saveCurrent();
		record_.clear();
	}
};





class EvalSubCtrl {
 public:
	EvalSubCtrl(const GameOptions& gameOptions, const TSOptions& mcts_options)
			: gameOptions_(gameOptions),
				logger_(elf::logging::getIndexedLogger(
							MAGENTA_B + std::string("|++|") + COLOR_END + 
							"EvalSubCtrl-", 
							"")) {
		// [TODO]: A bit hacky, we need to have a better way for this.
		mcts_opt_ = mcts_options;
		mcts_opt_.alg_opt.unexplored_q_zero = false;
		mcts_opt_.alg_opt.root_unexplored_q_zero = false;
		mcts_opt_.root_epsilon = 0.0;
		mcts_opt_.root_alpha = 0.0;
	}

	int64_t updateState(const ClientManager& mgr) {
		// Note that models_to_eval_ might change during the loop.
		// So we need to make a copy.
		std::lock_guard<std::mutex> lock(mutex_);
		auto models_to_eval = models_to_eval_;

		for (const auto& ver : models_to_eval) {
			ModelPerfomance& perf = find_or_create(mgr, get_key(ver));

			auto res = perf.updateState(mgr);
			switch (res) {
				case ModelPerfomance::EVAL_INVALID:
					logger_->info("res cannot be EVAL_INVALID");
					assert(false);
				case ModelPerfomance::EVAL_INCOMPLETE:
					break;
				case ModelPerfomance::EVAL_BLACK_PASS:
					// Check whether we need to make a conclusion.
					// Update reference.
					return perf.Pair().black_ver;
				case ModelPerfomance::EVAL_BLACK_NOTPASS:
					// In any case, pick the next model to evaluate.
					remove_candidate_model(perf.Pair().black_ver);
					break;
			}
		}
		// No new model.
		return -1;
	}

	FeedResult feedStats(const ClientInfo& info, const GameRecord& r) {
		if (r.request.vers.is_selfplay())
			return NOT_EVAL;

		std::lock_guard<std::mutex> lock(mutex_);

		ModelPerfomance* perf = find_or_null(r.request.vers);
		if (perf == nullptr)
			return NOT_REQUESTED;

		perf->feedInfo(info, r);
		return FEEDED;
	}

	int64_t getBestModel() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return best_baseline_model_;
	}

	void fillInRequest(const ClientInfo& info, MsgRequest* msg) {
		std::lock_guard<std::mutex> lock(mutex_);

		// Go through all current models
		// It uses the implicit heuristic that started from the oldest
		// model first.
		// Note that on_eval_status might change models_to_eval,
		for (const auto& ver : models_to_eval_) {
			ModelPerfomance& perf = find_or_create(info.getManager(), get_key(ver));
			perf.fillInRequest(info, msg);
			if (!msg->vers.wait())
				break;
		}
	}

	void setBaselineModel(int64_t ver) {
		std::lock_guard<std::mutex> lock(mutex_);

		best_baseline_model_ = ver;
		models_to_eval_.clear();
		// All perfs need to go away as well.
		// perfs_.clear();
	}

	void addNewModelForEvaluation(int64_t selfplay_ver, int64_t new_version) {
		std::lock_guard<std::mutex> lock(mutex_);

		if (selfplay_ver == best_baseline_model_) {
			if (selfplay_ver < new_version) {
				logger_->info(
						"Add new model for evaluation: {}, selfplay_ver: {}, baseline_ver: {}, eval_num_games={}\nTree Search Options :\n{}",
						new_version,
						selfplay_ver,
						best_baseline_model_,
						gameOptions_.eval_num_games,
						mcts_opt_.info(true));
				add_candidate_model(new_version);
			} else {
				logger_->warn(
						"New version: {} is the same or earlier than baseline: {}\nTree Search Options :\n{}",
						new_version,
						best_baseline_model_,
						mcts_opt_.info(true));
			}
		} else {
			logger_->warn(
					"New version {} is not registered. Selfplay_ver {} != internal one "
					"{}",
					new_version,
					selfplay_ver,
					best_baseline_model_);
		}
	}

 private:
	mutable std::mutex		mutex_;

	GameOptions 					gameOptions_;
	TSOptions							mcts_opt_;

	int64_t								best_baseline_model_ = -1;
	std::vector<int64_t>	models_to_eval_;

	std::unordered_map<ModelPair, std::unique_ptr<ModelPerfomance>> perfs_;

	std::shared_ptr<spdlog::logger> logger_;

	ModelPair get_key(int ver) {
		ModelPair p;
		
		p.black_ver = ver;
		p.white_ver = best_baseline_model_;
		p.mcts_opt = mcts_opt_;
		return p;
	}

	/*
		Add a model for evaluation. 
		Returns false if the model for eval has been added earlier.
	*/
	bool add_candidate_model(int ver) {
		auto it = perfs_.find(get_key(ver));
		if (it == perfs_.end())
			models_to_eval_.push_back(ver);
		return it == perfs_.end();
	}

	bool remove_candidate_model(int ver) {
		for (auto it = models_to_eval_.begin(); it != models_to_eval_.end(); ++it) {
			if (*it == ver) {
				models_to_eval_.erase(it);
				// We don't remove records in perfs_.
				return true;
			}
		}
		return false;
	}

	ModelPerfomance& find_or_create(const ClientManager& mgr, const ModelPair& mp) {
		auto it = perfs_.find(mp);

		if (it == perfs_.end()) {
			auto& ptr = perfs_[mp];
			ptr.reset(new ModelPerfomance(gameOptions_, mgr, mp));
			return *ptr;
		}
		return *it->second;
	}

	ModelPerfomance* find_or_null(const ModelPair& mp) {
		auto it = perfs_.find(mp);

		if (it == perfs_.end()) {
			logger_->info("The pair {} was not sent before!", mp.info());
			return nullptr;
		}
		return it->second.get();
	}
};
