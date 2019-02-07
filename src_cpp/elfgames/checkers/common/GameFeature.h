/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once


#include "elf/base/extractor.h"

#include "../checkers/CheckersState.h"
#include "../checkers/CheckersStateExt.h"

// enum SpecialActionType { SA_SKIP = -100, SA_PASS, SA_CLEAR };

class GameFeature {
 public:
	GameFeature(const CheckersGameOptions& options) : 
			options_(options) {
		_num_plane_checkers = CHECKERS_NUM_FEATURES;
	}
	// Inference part.
	static void extractCheckersState(
			const CheckersFeature& bf, 
			float* f) {
		bf.extract(f);
	}

	static void CheckersReplyValue(
			CheckersReply& reply, 
			const float* value) {
		reply.value = *value;
	}

	static void CheckersReplyPolicy(
			CheckersReply& reply, 
			const float* pi) {
		copy(pi, pi + reply.pi.size(), reply.pi.begin());
	}

	static void CheckersReplyAction(
			CheckersReply& reply, 
			const int64_t* action) {
		reply.c = *action;
	}

	static void CheckersReplyVersion(
			CheckersReply& reply, 
			const int64_t* ver) {
		reply.version = *ver;
	}


	// /////////////
	// // Training part.
	// посмотреть где используется в тренировке и как
	static void extractCheckersMoveIdx(
			const CheckersStateExtOffline& s, 
			int* move_idx) {

		// просто текущий номер хода
		// (при приходе доски он проигрывает игры согласно JSON)
		// и итерирует move_idx

		*move_idx = s._state.getPly() - 1;
	}

	static void extractCheckersNumMove(
			const CheckersStateExtOffline& s, 
			int* num_move) {
		*num_move = s.getNumMoves();
	}

	static void extractCheckersPredictedValue(
			const CheckersStateExtOffline& s, 
			float* predicted_value) {
		*predicted_value = s.getPredictedValue(s._state.getPly() - 1);
	}


	static void extractCheckersWinner(
			const CheckersStateExtOffline& s, 
			float* winner) {
		*winner = s._offline_winner;
	}


	// Посмотреть где вызывается
	static void extractCheckersStateExt(
			const CheckersStateExtOffline& s, 
			float* f) {
		// Then send the data to the server.
		extractCheckersState(s._bf, f);
	}

	// Проверить!!!!!!!!!!!!!!!!!!!!!
	static void extractCheckersMCTSPi(
			const CheckersStateExtOffline& s, 
			float* mcts_scores) {

		const size_t move_to = s._state.getPly() - 1;

		// std::cout << "move_to : " << move_to << std::endl;
		// std::cout << "TOTAL_NUM_ACTIONS : " << TOTAL_NUM_ACTIONS << std::endl;
		// std::cout << "s._mcts_policies.size() : " << s._mcts_policies.size() << std::endl;

		std::fill(mcts_scores, mcts_scores + TOTAL_NUM_ACTIONS, 0.0);
		if (move_to < s._mcts_policies.size()) {
			const auto& policy = s._mcts_policies[move_to].prob;
			float sum_v = 0.0;
			for (size_t i = 0; i < TOTAL_NUM_ACTIONS; ++i) {
				mcts_scores[i] = policy[i];
				sum_v += mcts_scores[i];
			}
			// Then we normalize.
			for (size_t i = 0; i < TOTAL_NUM_ACTIONS; ++i) {
				mcts_scores[i] /= sum_v;
			}
		} else {
			mcts_scores[s._offline_all_moves[move_to]] = 1.0;
		}
	}

	static void extractCheckersOfflineAction(
			const CheckersStateExtOffline& s, 
			int64_t* offline_a) {
		const CheckersFeature& bf = s._bf;

		std::fill(offline_a, offline_a + s._options.checkers_num_future_actions, 0);
		const size_t move_to = s._state.getPly() - 1;
		for (int i = 0; i < s._options.checkers_num_future_actions; ++i) {
			Coord m = s._offline_all_moves[move_to + i];
			offline_a[i] = m;
		}
	}

	static void extractCheckersStateSelfplayVersion(
			const CheckersStateExtOffline& s, 
			int64_t* ver) {
		*ver = s._curr_request.vers.black_ver;
	}

	static void extractCheckersAIModelBlackVersion(
			const ModelPair& msg, 
			int64_t* ver) {
		*ver = msg.black_ver;
	}

	static void extractCheckersAIModelWhiteVersion(
			const ModelPair& msg, 
			int64_t* ver) {
		*ver = msg.white_ver;
	}

	static void extractCheckersSelfplayVersion(
			const MsgVersion& msg, 
			int64_t* ver) {
		*ver = msg.model_ver;
	}

	// ================================================================
	// ================================================================
	void registerExtractor(int batchsize, elf::Extractor& e) {
		display_debug_info("GameFeature", __FUNCTION__, RED_B);

		// регаем ключ по которому мы будем доставать наш state и сообщаем размерность, 
		// что он будет такой размерности
		auto& checkers_s = e.addField<float>("checkers_s").addExtents(
			batchsize, {batchsize, _num_plane_checkers, CHECKERS_BOARD_SIZE, CHECKERS_BOARD_SIZE});
		// добавляем к этому ключу вот методы для 
		// тк нам нужно заполнить эту память и передать python
		checkers_s.addFunction<CheckersFeature>(extractCheckersState)
			.addFunction<CheckersStateExtOffline>(extractCheckersStateExt);


		// добавляем еще поля 
		e.addField<int64_t>("a").addExtent(batchsize);
		e.addField<int64_t>("checkers_rv").addExtent(batchsize);
		e.addField<int64_t>("checkers_offline_a")
				.addExtents(batchsize, {batchsize, options_.checkers_num_future_actions});
		e.addField<float>({
			"checkers_V", 
			"checkers_winner", 
			"checkers_predicted_value"}).addExtent(batchsize);
		e.addField<float>({"pi", "checkers_mcts_scores"})
				.addExtents(batchsize, {batchsize, TOTAL_NUM_ACTIONS});
		e.addField<int32_t>({"checkers_move_idx", "checkers_aug_code", "checkers_num_move"})
				.addExtent(batchsize);
		e.addField<int64_t>({"checkers_black_ver", "checkers_white_ver", "checkers_selfplay_ver"})
				.addExtent(batchsize);



		// привязываем к каждому полю свой метод для записи инфы в код ++
		e.addClass<CheckersReply>()
				.addFunction<int64_t>("a", CheckersReplyAction)
				.addFunction<float>("pi", CheckersReplyPolicy)
				.addFunction<float>("checkers_V", CheckersReplyValue)
				.addFunction<int64_t>("checkers_rv", CheckersReplyVersion);

		e.addClass<CheckersStateExtOffline>()
				.addFunction<int32_t>("checkers_move_idx", extractCheckersMoveIdx)
				.addFunction<int32_t>("checkers_num_move", extractCheckersNumMove)
				.addFunction<float>("checkers_predicted_value", extractCheckersPredictedValue)
				.addFunction<float>("checkers_winner", extractCheckersWinner)
				.addFunction<float>("checkers_mcts_scores", extractCheckersMCTSPi)
				.addFunction<int64_t>("checkers_offline_a", extractCheckersOfflineAction)
				.addFunction<int64_t>("checkers_selfplay_ver", extractCheckersStateSelfplayVersion)
				;

		e.addClass<ModelPair>()
				.addFunction<int64_t>("checkers_black_ver", extractCheckersAIModelBlackVersion)
				.addFunction<int64_t>("checkers_white_ver", extractCheckersAIModelWhiteVersion);

		e.addClass<MsgVersion>().addFunction<int64_t>(
				"checkers_selfplay_ver", extractCheckersSelfplayVersion);

	}


	std::map<std::string, int> getParams() const {
		display_debug_info("GameFeature", __FUNCTION__, RED_B);
		
		return std::map<std::string, int>{
				{"checkers_num_action", TOTAL_NUM_ACTIONS},
				{"checkers_board_size", 8},
				{"checkers_num_future_actions", 1},
				{"checkers_num_planes", _num_plane_checkers}
		};
	}

 private:
 	// Параметры игры
	CheckersGameOptions options_;
	// Количество фич доски
	int 								_num_plane_checkers;
};