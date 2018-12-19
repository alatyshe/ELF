/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "../base/go_state.h"
#include "go_game_specific.h"
#include "go_state_ext.h"

#include "elf/base/extractor.h"

enum SpecialActionType { SA_SKIP = -100, SA_PASS, SA_CLEAR };

class GoFeature {
 public:
  GoFeature(const GameOptions& options) : options_(options) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    if (options.use_df_feature) {
      _num_plane = MAX_NUM_FEATURE;
      _our_stone_plane = OUR_STONES;
      _opponent_stone_plane = OPPONENT_STONES;
    } else {
      _num_plane = MAX_NUM_AGZ_FEATURE;
      _our_stone_plane = 0;
      _opponent_stone_plane = 1;
    }
  }

  // Inference part.
  static void extractState(const BoardFeature& bf, float* f) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    bf.extract(f);
  }


  static void ReplyValue(GoReply& reply, const float* value) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    reply.value = *value;
  }

  static void ReplyPolicy(GoReply& reply, const float* pi) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    copy(pi, pi + reply.pi.size(), reply.pi.begin());
  }

  static void ReplyAction(GoReply& reply, const int64_t* action) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    switch ((SpecialActionType)*action) {
      case SA_SKIP:
        reply.c = M_SKIP;
        break;
      case SA_PASS:
        reply.c = M_PASS;
        break;
      case SA_CLEAR:
        reply.c = M_CLEAR;
        break;
      default:
        reply.c = reply.bf.action2Coord(*action);
    }
  }

  static void ReplyVersion(GoReply& reply, const int64_t* ver) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    reply.version = *ver;
  }

  /////////////
  // Training part.
  static void extractMoveIdx(const GoStateExtOffline& s, int* move_idx) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);
    // просто текущий номер хода
    // (при приходе доски он проигрывает игры согласно JSON)
    // и итерирует move_idx

    *move_idx = s._state.getPly() - 1;
  }

  static void extractNumMove(const GoStateExtOffline& s, int* num_move) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    *num_move = s.getNumMoves();

    std::cout << "extractNumMove\t: " << *num_move << std::endl;
  }

  static void extractPredictedValue(
      const GoStateExtOffline& s,
      float* predicted_value) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    *predicted_value = s.getPredictedValue(s._state.getPly() - 1);

    std::cout << "extractPredictedValue : " << *predicted_value << std::endl;
  }


  static void extractWinner(const GoStateExtOffline& s, float* winner) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    *winner = s._offline_winner;
  }

  static void extractStateExt(const GoStateExtOffline& s, float* f) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    // std::cout << "extractStateExt : " << std::endl;
    // std::cout << s._state.showBoard() << std::endl;
    // Then send the data to the server.
    extractState(s._bf, f);
  }

  static void extractMCTSPi(const GoStateExtOffline& s, float* mcts_scores) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    std::cout << "extractMCTSPi : " << std::endl;

    const BoardFeature& bf = s._bf;
    const size_t move_to = s._state.getPly() - 1;

    std::cout << "move_to : " << move_to << std::endl;
    std::cout << "BOARD_NUM_ACTION : " << BOARD_NUM_ACTION << std::endl;
    std::cout << "s._mcts_policies.size() : " << s._mcts_policies.size() << std::endl;

    std::fill(mcts_scores, mcts_scores + BOARD_NUM_ACTION, 0.0);

    if (move_to < s._mcts_policies.size()) {
      std::cout << "IF1" << std::endl;

      const auto& policy = s._mcts_policies[move_to].prob;
      float sum_v = 0.0;
      for (size_t i = 0; i < BOARD_NUM_ACTION; ++i) {
        mcts_scores[i] = policy[bf.action2Coord(i)];

        std::cout << "mcts_scores[" << i << "] : " << mcts_scores[i] << std::endl;
        sum_v += mcts_scores[i];
      }
      std::cout << "sum_v : " << sum_v << std::endl;
      // Then we normalize.
      for (size_t i = 0; i < BOARD_NUM_ACTION; ++i) {
        mcts_scores[i] /= sum_v;
      }
    } else {
      std::cout << "ELSE2" << std::endl;

      mcts_scores[bf.coord2Action(s._offline_all_moves[move_to])] = 1.0;
    }
    std::cout << std::endl << std::endl;
  }

  static void extractOfflineAction(
      const GoStateExtOffline& s,
      int64_t* offline_a) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    const BoardFeature& bf = s._bf;

    // std::cout << "extractOfflineAction : " << std::endl;

    // std::cout << s._options.info() << std::endl << std::endl;

    std::cout << "s._options.num_future_actions : " << s._options.num_future_actions << std::endl;

    std::fill(offline_a, offline_a + s._options.num_future_actions, 0);
    const size_t move_to = s._state.getPly() - 1;
    for (int i = 0; i < s._options.num_future_actions; ++i) {
      Coord m = s._offline_all_moves[move_to + i];
      offline_a[i] = bf.coord2Action(m);
      std::cout << " i : " << i << "\tm : " << m << std::endl;
    }
    std::cout << std::endl << std::endl << std::endl;
  }

  static void extractStateSelfplayVersion(
      const GoStateExtOffline& s,
      int64_t* ver) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    *ver = s.curr_request_.vers.black_ver;
  }

  static void extractAIModelBlackVersion(const ModelPair& msg, int64_t* ver) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    *ver = msg.black_ver;

    std::cout << "extractAIModelBlackVersion : " << *ver << std::endl;
  }

  static void extractAIModelWhiteVersion(const ModelPair& msg, int64_t* ver) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    *ver = msg.white_ver;

    std::cout << "extractAIModelWhiteVersion : " << *ver << std::endl;
  }

  static void extractSelfplayVersion(const MsgVersion& msg, int64_t* ver) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    *ver = msg.model_ver;

    std::cout << "extractSelfplayVersion : " << *ver << std::endl;
  }

  void registerExtractor(int batchsize, elf::Extractor& e) {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);

    // Register multiple fields.
    auto& s = e.addField<float>("s").addExtents(
        batchsize, {batchsize, _num_plane, BOARD_SIZE, BOARD_SIZE});
    s.addFunction<BoardFeature>(extractState)
      .addFunction<GoStateExtOffline>(extractStateExt);

    e.addField<int64_t>("a").addExtent(batchsize);
    e.addField<int64_t>("rv").addExtent(batchsize);
    e.addField<int64_t>("offline_a")
        .addExtents(batchsize, {batchsize, options_.num_future_actions});
    e.addField<float>({"V", "winner", "predicted_value"}).addExtent(batchsize);
    e.addField<float>({"pi", "mcts_scores"})
        .addExtents(batchsize, {batchsize, BOARD_NUM_ACTION});
    e.addField<int32_t>({"move_idx", "aug_code", "num_move"})
        .addExtent(batchsize);

    e.addField<int64_t>({"black_ver", "white_ver", "selfplay_ver"})
        .addExtent(batchsize);

    e.addClass<GoReply>()
        .addFunction<int64_t>("a", ReplyAction)
        .addFunction<float>("pi", ReplyPolicy)
        .addFunction<float>("V", ReplyValue)
        .addFunction<int64_t>("rv", ReplyVersion);

    e.addClass<GoStateExtOffline>()
        .addFunction<int32_t>("move_idx", extractMoveIdx)
        .addFunction<int32_t>("num_move", extractNumMove)
        .addFunction<float>("predicted_value", extractPredictedValue)
        .addFunction<float>("winner", extractWinner)
        .addFunction<float>("mcts_scores", extractMCTSPi)
        .addFunction<int64_t>("offline_a", extractOfflineAction)
        .addFunction<int64_t>("selfplay_ver", extractStateSelfplayVersion);

    e.addClass<ModelPair>()
        .addFunction<int64_t>("black_ver", extractAIModelBlackVersion)
        .addFunction<int64_t>("white_ver", extractAIModelWhiteVersion);

    e.addClass<MsgVersion>().addFunction<int64_t>(
        "selfplay_ver", extractSelfplayVersion);
  }

  std::map<std::string, int> getParams() const {
    display_debug_info("GoFeature", __FUNCTION__, RED_B);
    
    return std::map<std::string, int>{
        {"num_action", BOARD_NUM_ACTION},
        {"board_size", BOARD_SIZE},
        {"num_future_actions", options_.num_future_actions},
        {"num_planes", _num_plane},
        {"our_stone_plane", _our_stone_plane},
        {"opponent_stone_plane", _opponent_stone_plane},
        {"ACTION_SKIP", SA_SKIP},
        {"ACTION_PASS", SA_PASS},
        {"ACTION_CLEAR", SA_CLEAR},
    };
  }

 private:
  GameOptions options_;

  int _num_plane;
  int _our_stone_plane;
  int _opponent_stone_plane;
};
