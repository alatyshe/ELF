/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory>
#include <random>
#include <string>

#include "elf/base/dispatcher.h"
#include "elf/legacy/python_options_utils_cpp.h"
#include "elf/logging/IndexedLoggerFactory.h"

#include "../mcts/MCTS.h"
#include "../sgf/sgf.h"
#include "game_base.h"
#include "game_feature.h"
#include "game_stats.h"
#include "notifier.h"

#include "../checkers/CheckersStateExt.h"
#include "../checkers/CheckersFeature.h"
#include "../checkers/CheckersState.h"
// #include "../checkers/Notifier.h"

class AIClientT;

// Game interface for Go.
class GameSelfPlay : public GameBase {
 public:
  using ThreadedDispatcher = elf::ThreadedDispatcherT<MsgRequest, RestartReply>;
  GameSelfPlay(
      int game_idx,
      elf::GameClient* client,
      const ContextOptions& context_options,
      const GameOptions& options,
      ThreadedDispatcher* dispatcher,
      CheckersGameNotifierBase* checkers_notifier = nullptr);

  void act() override;
  bool OnReceive(const MsgRequest& request, RestartReply* reply);

  std::string showBoard() const {
    display_debug_info("GameSelfPlay", __FUNCTION__, RED_B);
    // std::cout << _checkers_state_ext.state().showBoard() << std::endl << std::endl;

    return _checkers_state_ext.state().showBoard();
  }

  // std::string getNextPlayer() const {
  //   display_debug_info("GameSelfPlay", __FUNCTION__, RED_B);

  //   if (_checkers_state_ext.state().nextPlayer() == BLACK_PLAYER)
  //     return "Black";
  //   return "White";
  // }




  std::string getLastMove() const {
    display_debug_info("GameSelfPlay", __FUNCTION__, RED_B);
    
    return std::to_string(_checkers_state_ext.lastMove());
  }

  // float getScore() {
  //   display_debug_info("GameSelfPlay", __FUNCTION__, RED_B);
  //   std::cout << _state_ext.state().evaluate(_options.komi) << std::endl << std::endl;

  //   return _state_ext.state().evaluate(_options.komi);
  // }


  std::array<int, ALL_ACTIONS>    GetValidMoves() const {
    return GetValidMovesBinary(_checkers_state_ext.state().board(), 
      _checkers_state_ext.state().board().active);
  }


  float GetScore() {
    display_debug_info("GameSelfPlay", __FUNCTION__, RED_B);
    // std::cout << _checkers_state_ext.state().evaluate() << std::endl << std::endl;

    return _checkers_state_ext.state().evaluate();
  }


  // float getLastScore() const {
  //   display_debug_info("GameSelfPlay", __FUNCTION__, RED_B);
  //   std::cout << _state_ext.getLastGameFinalValue() << std::endl << std::endl;

  //   return _state_ext.getLastGameFinalValue();
  // }

 private:
  void setAsync();
  void restart();


  MCTSCheckersAI* init_checkers_ai(
      const std::string& actor_name,
      const elf::ai::tree_search::TSOptions& mcts_opt,
      float second_puct,
      int second_mcts_rollout_per_batch,
      int second_mcts_rollout_per_thread,
      int64_t model_ver);


  Coord mcts_make_diverse_move(MCTSCheckersAI* mcts_checkers_ai, Coord c);
  Coord mcts_update_info(MCTSCheckersAI* mcts_checkers_ai, Coord c);
  
  void  finish_game(CheckersFinishReason reason);
  
  

 private:
  ThreadedDispatcher*         dispatcher_ = nullptr;
  // My
  CheckersGameNotifierBase*   checkers_notifier_ = nullptr;
  CheckersStateExt            _checkers_state_ext;


  int _online_counter = 0;

  std::unique_ptr<MCTSCheckersAI> checkers_ai1;
  // Opponent ai (used for selfplay evaluation)
  std::unique_ptr<MCTSCheckersAI> checkers_ai2;

  std::unique_ptr<CheckersAI> _human_player;

  std::shared_ptr<spdlog::logger> logger_;
};
