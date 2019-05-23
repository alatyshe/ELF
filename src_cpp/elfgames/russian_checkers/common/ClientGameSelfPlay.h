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
#include "Notifier.h"

#include "../game/CheckersStateExt.h"
#include "../game/CheckersFeature.h"
#include "../game/CheckersState.h"

/*
  Running on the client side to generate batches.
  Contains:
  ThreadedDispatcher - checks messages from server(update model version, eval 2 models etc).
  CheckersGameNotifierBase - Used to call the game_end python function from selfplay.py
      Displays statistics about the game from python side and contain records of finished games.
  CheckersStateExt - Generates batches, dumps finished games to records(json) etc.
  checkers_ai1 - uses MCTS for searching best action. Responsible for
      generating batches for training the neural network.
  checkers_ai2 - also uses MCTS. Initialized only when the client receives
      a notification from the server to compare two models.
  _human_player - The base class AIClientT that sends batch files from C++ to python 
      and expects to receive an answer. In our case, these are the keys that 
      we registered in the GameFeature.h and game.py files namely by 
      "pi", "a", "checkers_V".
  logger_ - displays log info in terminal.
*/
class ClientGameSelfPlay : public GameBase {
 public:
  using ThreadedDispatcher = elf::ThreadedDispatcherT<MsgRequest, RestartReply>;
  
  ClientGameSelfPlay(
      int game_idx,
      elf::GameClient* client,
      const ContextOptions& context_options,
      const CheckersGameOptions& game_options,
      ThreadedDispatcher* dispatcher,
      CheckersGameNotifierBase* checkers_notifier = nullptr);

  bool OnReceive(const MsgRequest& request, RestartReply* reply);

  void act() override;
  std::string showBoard() const;
  // returns all valid moves for current board
  std::array<int, TOTAL_NUM_ACTIONS> getValidMoves() const;
  float getScore();


  // Gui
  std::array<std::array<int, 8>, 8> getBoard() const;
  int getCurrentPlayer() const;

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
  void finish_game();

 private:
  ThreadedDispatcher* dispatcher_ = nullptr;
  CheckersGameNotifierBase* checkers_notifier_ = nullptr;
  CheckersStateExt _checkers_state_ext;

  int _online_counter = 0;
  std::unique_ptr<MCTSCheckersAI> checkers_ai1;
  // Opponent ai (used for selfplay evaluation)
  std::unique_ptr<MCTSCheckersAI> checkers_ai2;
  
  std::unique_ptr<AIClientT> _human_player;

  std::shared_ptr<spdlog::logger> logger_;
};
