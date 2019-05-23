/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>

// elf
#include "elf/ai/tree_search/mcts.h"
#include "elf/logging/IndexedLoggerFactory.h"
// game
#include "AI.h"

struct MCTSActorParams {
	// "actor_black", "actor_white"
	std::string		actor_name;

	uint64_t			seed = 0;
	// Required model version.
	// If -1, then there is no requirement on model version (any model response
	// can be used).
	int64_t				required_version = -1;

	std::string info() const {
		std::stringstream ss;
		ss  << "[name=" << actor_name << "][seed=" 
				<< seed << "][requred_ver=" << required_version << "]";
		return ss.str();
	}
};


/*
	Only agent. used to send states to neural network for evaluation
	and other methods for MCTS.
*/
class MCTSGameActor {
 public:
	using Action = Coord;
	using State	= GameState;
	using NodeResponse = elf::ai::tree_search::NodeResponseT<Coord>;

	enum PreEvalResult { EVAL_DONE, EVAL_NEED_NN };

	MCTSGameActor(elf::GameClient* client, const MCTSActorParams& params)
			: params_(params),
				rng_(params.seed),
				logger_(elf::logging::getIndexedLogger(
					MAGENTA_B + std::string("|++|") + COLOR_END + 
					"MCTSGameActor-", 
					"")) {
		ai_.reset(new AIClientT(client, {params_.actor_name}));

		// logger_->info(
		// 		"MCTS Actor params : {}", params.info());
	}

	std::string info() const {
		return params_.info();
	}

	void set_ostream(std::ostream* oo) {
		oo_ = oo;
	}

	void setRequiredVersion(int64_t ver) {
		params_.required_version = ver;
	}

	std::mt19937* rng() {
		return &rng_;
	}

	void evaluate(
			const std::vector<const GameState*>& states,
			std::vector<NodeResponse>* p_resps) {
		if (states.empty())
			return;

		if (oo_ != nullptr)
			*oo_ 	<< std::endl << std::endl << "Evaluating batch state. #states: " 
						<< states.size() << std::endl;

		auto& resps = *p_resps;

		resps.resize(states.size());
		std::vector<BoardFeature> sel_bfs;
		std::vector<size_t> sel_indices;

		for (size_t i = 0; i < states.size(); i++) {
			assert(states[i] != nullptr);
			PreEvalResult res = pre_evaluate(*states[i], &resps[i]);
			if (res == EVAL_NEED_NN) {
				sel_bfs.push_back(get_extractor(*states[i]));
				sel_indices.push_back(i);
			}
		}

		if (sel_bfs.empty())
			return;

		std::vector<BoardReply> replies;
		for (size_t i = 0; i < sel_bfs.size(); ++i) {
			replies.emplace_back(sel_bfs[i]);
		}

		// Get all pointers.
		std::vector<BoardReply*> p_replies;
		std::vector<const BoardFeature*> p_bfs;

		for (size_t i = 0; i < sel_bfs.size(); ++i) {
			p_bfs.push_back(&sel_bfs[i]);
			p_replies.push_back(&replies[i]);
		}

		// will call neural net
		if (!ai_->act_batch(p_bfs, p_replies)) {
			logger_->info("act unsuccessful! ");
		} else {
			for (size_t i = 0; i < sel_indices.size(); i++) {
				post_nn_result(replies[i], &resps[sel_indices[i]]);
			}
		}
	}
	void evaluate(const GameState& s, NodeResponse* resp) {
		if (oo_ != nullptr)
			*oo_ 	<< std::endl << std::endl << "Evaluating state at " 
						<< std::hex << &s << std::dec << std::endl;

		// if terminated(), get results, res = done
		// else res = EVAL_NEED_NN
		PreEvalResult res = pre_evaluate(s, resp);

		if (res == EVAL_NEED_NN) {
			BoardFeature bf = get_extractor(s);
			// BoardReply struct initialization
			// members containing:
			// Coord c, vector<float> pi, float v;
			BoardReply reply(bf);

			// AI-Client will run a one-step neural network
			// will call neural net
			if (!ai_->act(bf, &reply)) {
				// This happens when the game is about to end,
				logger_->info("act unsuccessful! ");
			} else {
				// call pi2response()
				// action will be inv-transformed
				post_nn_result(reply, resp);
			}
		}

		if (oo_ != nullptr)
			*oo_ << "Finish evaluating state at " << std::hex << &s << std::dec
					 << std::endl;
	}


	bool forward(GameState& s, Coord a) {
		return s.forward(a);
	}


	void setID(int id) {
		ai_->setID(id);
	}


	float reward(const GameState& /*s*/, float value) const {
		return value;
	}

 protected:
	MCTSActorParams params_;
	
	// client to run neural network
	std::unique_ptr<AIClientT> ai_;
	std::ostream* oo_ = nullptr;
	std::mt19937 rng_;

 private:
	std::shared_ptr<spdlog::logger> logger_;
	
	BoardFeature get_extractor(const GameState& s) {
		return BoardFeature(s);
	}

	/*
		check terminated;
		Whether we need to call the neural network to evaluate 
		the next state of the board, or we can already get a reward.
	*/
	PreEvalResult pre_evaluate(const GameState& s, NodeResponse* resp) {
		resp->q_flip = s.currentPlayer() == WHITE_PLAYER;

		if (s.terminated()) {
			float final_value = s.evaluateGame();
	
			if (oo_ != nullptr) {
				*oo_ << s.showBoard() << std::endl;
				*oo_ << "Terminal state at " << s.getPly() << std::endl;
				*oo_ << "Score :" << final_value << std::endl; 
				*oo_ << "Moves[" << s.getAllMoves().size()
						 << "]: " << s.getAllMovesString() << std::endl;
			}
			resp->value = final_value > 0 ? 1.0 : -1.0;
			// No further action.
			resp->pi.clear();
			return EVAL_DONE;
		} else {
			return EVAL_NEED_NN;
		}
	}

	
	void post_nn_result(const BoardReply& reply, NodeResponse* resp) {
		if (params_.required_version >= 0 &&
				reply.version != params_.required_version) {
			const std::string msg = "model version " + std::to_string(reply.version) +
					" and required version " + std::to_string(params_.required_version) +
					" are not consistent";
			logger_->error(msg);
			throw std::runtime_error(msg);
		}

		if (oo_ != nullptr)
			*oo_ << "Got information from neural network" << std::endl;
		resp->value = reply.value;

		const GameState& s = reply.bf.state();
		pi2response(reply.bf, reply.pi, &resp->pi, oo_);
	}

	
	static void normalize(std::vector<std::pair<Coord, float>>* output_pi) {
		assert(output_pi != nullptr);
		float total_prob = 1e-10;

		for (const auto& p : *output_pi) {
			total_prob += p.second;
		}

		for (auto& p : *output_pi) {
			p.second /= total_prob;
		}
	}


	// with inv-transform considered, remove invalid moves, normalize
	// output_pi
	static void pi2response(
			const BoardFeature& bf,
			const std::vector<float>& pi,
			std::vector<std::pair<Coord, float>>* output_pi,
			std::ostream* oo = nullptr) {
		const GameState& s = bf.state();

		if (oo != nullptr) {
			*oo << "In get_last_pi, #move returned: " << pi.size() << std::endl;
			*oo << s.showBoard() << std::endl;
		}

		output_pi->clear();

		// No action for terminated state.
		if (s.terminated()) {
			if (oo != nullptr)
				*oo << "Terminal state at " << s.getPly() << std::endl;
			return;
		}

		for (size_t i = 0; i < pi.size(); ++i) {
			// Inv random transform will be applied
			Coord m = i;
			// 
			// if (oo != nullptr) {
			// 	*oo << "  Action " << std::setw(3) << std::left << i << " to Coord "
			// 			<< moves::m_to_h.find(i)->second
			// 			<< std::endl;
			// }

			output_pi->push_back(std::make_pair(m, pi[i]));
		}
		// // sorting..
		using data_type = std::pair<Coord, float>;

		if (oo != nullptr)
			*oo << "Before sorting moves" << std::endl;

		std::sort(
				output_pi->begin(),
				output_pi->end(),
				[](const data_type& d1, const data_type& d2) {
					return d1.second > d2.second;
				});

		if (oo != nullptr)
			*oo << "After sorting moves" << std::endl;

		std::vector<data_type> tmp;
		int i = 0;
		while (true) {
			if (i >= (int)output_pi->size())
				break;
			const data_type& v = output_pi->at(i);
			// Check whether this move is right.
			bool valid = s.checkMove(v.first);
			if (valid) {
				tmp.push_back(v);
			}

			if (oo != nullptr) {
				// *oo << "Predict [" << i << "][" << v.first << "] " << v.second;
				if (valid) {
					*oo << "Predict [" << std::setw(3) << std::right << i 
							<< "][" << std::setw(3) << std::right << v.first << "] "
							<< std::setw(8) << std::right << moves::m_to_h.find(v.first)->second << " "
							<< v.second;
					*oo << " added" << std::endl;
				}
				// else
				// 	*oo << " invalid" << std::endl;
			}
			i++;
		}
		*output_pi = tmp;
		normalize(output_pi);
		if (oo != nullptr)
			*oo << "Total valid moves: " << output_pi->size() << std::endl << std::endl;
	}
};


