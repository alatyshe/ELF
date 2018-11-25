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

#include "../mcts/mcts.h"
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
class GoGameSelfPlay : public GoGameBase {
 public:
  using ThreadedDispatcher = elf::ThreadedDispatcherT<MsgRequest, RestartReply>;
  GoGameSelfPlay(
      int game_idx,
      elf::GameClient* client,
      const ContextOptions& context_options,
      const GameOptions& options,
      ThreadedDispatcher* dispatcher,
      GameNotifierBase* notifier = nullptr);

  void act() override;
  bool OnReceive(const MsgRequest& request, RestartReply* reply);

  std::string showBoard() const {
    display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);
    // std::cout << _checkers_state_ext.state().showBoard() << std::endl << std::endl;

    return _state_ext.state().showBoard();
  }

  // std::string getNextPlayer() const {
  //   display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);
  //   std::cout << player2str(_state_ext.state().nextPlayer()) << std::endl << std::endl;

  //   return player2str(_state_ext.state().nextPlayer());
  // }

  // std::string getLastMove() const {
  //   display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);
  //   std::cout << coord2str2(_state_ext.lastMove()) << std::endl << std::endl;

  //   return coord2str2(_state_ext.lastMove());
  // }

  float getScore() {
    display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);
    std::cout << _state_ext.state().evaluate(_options.komi) << std::endl << std::endl;

    return _state_ext.state().evaluate(_options.komi);
  }

  // float getLastScore() const {
  //   display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);
  //   std::cout << _state_ext.getLastGameFinalValue() << std::endl << std::endl;


  //   return _state_ext.getLastGameFinalValue();
  // }

 private:
  void setAsync();
  void restart();

  MCTSGoAI* init_ai(
      const std::string& actor_name,
      const elf::ai::tree_search::TSOptions& mcts_opt,
      float second_puct,
      int second_mcts_rollout_per_batch,
      int second_mcts_rollout_per_thread,
      int64_t model_ver);
  Coord mcts_make_diverse_move(MCTSGoAI* curr_ai, Coord c);
  Coord mcts_update_info(MCTSGoAI* mcts_go_ai, Coord c);
  void finish_game(FinishReason reason);

 private:
  ThreadedDispatcher* dispatcher_ = nullptr;
  GameNotifierBase*   notifier_ = nullptr;
  GoStateExt          _state_ext;
  
  // My
  // CheckersGameNotifier*   checkers_notifier_ = nullptr;
  CheckersStateExt        _checkers_state_ext;


  int _online_counter = 0;

  std::unique_ptr<MCTSGoAI> go_ai1;
  // Opponent ai (used for selfplay evaluation)
  std::unique_ptr<MCTSGoAI> go_ai2;
  std::unique_ptr<AI> _human_player;


  // std::unique_ptr<AIClientT> _checkers_ai;

  std::shared_ptr<spdlog::logger> logger_;
};
