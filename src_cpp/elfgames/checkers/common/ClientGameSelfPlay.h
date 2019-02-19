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

// elf
#include "elf/base/dispatcher.h"
#include "elf/legacy/python_options_utils_cpp.h"
#include "elf/logging/IndexedLoggerFactory.h"

// checkers
#include "../mcts/AI.h"
#include "../mcts/CheckersMCTSActor.h"
#include "../mcts/MCTSCheckersAI.h"
#include "../sgf/sgf.h"
#include "GameBase.h"
#include "GameFeature.h"
#include "GameStats.h"
#include "notifier.h"

#include "../checkers/CheckersStateExt.h"
#include "../checkers/CheckersFeature.h"
#include "../checkers/CheckersState.h"

// ==========================================================
// ==========================================================
class ClientGameSelfPlay : public GameBase {
 public:
  using ThreadedDispatcher = elf::ThreadedDispatcherT<MsgRequest, RestartReply>;
  ClientGameSelfPlay(
      int                         game_idx,
      elf::GameClient*            client,
      const ContextOptions&       context_options,
      const CheckersGameOptions&  game_options,
      ThreadedDispatcher*         dispatcher,
      CheckersGameNotifierBase*   checkers_notifier = nullptr);

  bool OnReceive(const MsgRequest& request, RestartReply* reply);

  void act() override;
  std::string showBoard() const;
  std::array<int, TOTAL_NUM_ACTIONS> getValidMoves() const;
  float getScore();

 private:
  MCTSCheckersAI* init_checkers_ai(
      const std::string& actor_name,
      const elf::ai::tree_search::TSOptions& mcts_opt,
      float second_puct,
      int second_mcts_rollout_per_batch,
      int second_mcts_rollout_per_thread,
      int64_t model_ver);

  void restart();
  void setAsync();
  Coord mcts_make_diverse_move(MCTSCheckersAI* mcts_checkers_ai, Coord c);
  Coord mcts_update_info(MCTSCheckersAI* mcts_checkers_ai, Coord c);
  void finish_game(CheckersFinishReason reason);
  
  

 private:
  ThreadedDispatcher*             dispatcher_ = nullptr;
  CheckersGameNotifierBase*       checkers_notifier_ = nullptr;
  CheckersStateExt                _checkers_state_ext;


  int _online_counter = 0;

  std::unique_ptr<MCTSCheckersAI> checkers_ai1;
  // Opponent ai (used for selfplay evaluation)
  std::unique_ptr<MCTSCheckersAI> checkers_ai2;
  
  std::unique_ptr<AIClientT>     _human_player;

  std::shared_ptr<spdlog::logger> logger_;
};
