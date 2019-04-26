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

#include "../mcts/AI.h"
#include "../mcts/MCTSGameActor.h"
#include "../mcts/MCTSGameAI.h"
#include "../sgf/sgf.h"
#include "GameBase.h"

#include "GameFeature.h"
#include "GameStats.h"
#include "Notifier.h"

#include "../game/GameStateExt.h"
#include "../game/BoardFeature.h"
#include "../game/GameState.h"
#include "../game/SimpleAgent.h"

/*
  Running on the client side to generate batches.
  Contains:
  ThreadedDispatcher - checks messages from server(update model version, eval 2 models etc).
  GameNotifierBase - Used to call the game_end python function from selfplay.py
      Displays statistics about the game from python side and contain records of finished games.
  GameStateExt - Generates batches, dumps finished games to records(json) etc.
  ai1_ - uses MCTS for searching best action. Responsible for
      generating batches for training the neural network.
  ai2_ - also uses MCTS. Initialized only when the client receives
      a notification from the server to compare two models.
  human_player_ - The base class AIClientT that sends batch files from C++ to python 
      and expects to receive an answer. In our case, these are the keys that 
      we registered in the GameFeature.h and game.py files namely by 
      "pi", "a", "V".
  logger_ - displays log info in terminal.
*/
class ClientGameSelfPlay : public GameBase {
 public:
  using ThreadedDispatcher = elf::ThreadedDispatcherT<MsgRequest, RestartReply>;
  
  ClientGameSelfPlay(
      int game_idx,
      elf::GameClient* client,
      const ContextOptions& context_options,
      const GameOptions& game_options,
      ThreadedDispatcher* dispatcher,
      GameNotifierBase* gameNotifier = nullptr);

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
  MCTSGameAI* init_ai(
      const std::string& actor_name,
      const elf::ai::tree_search::TSOptions& mcts_opt,
      float second_puct,
      int second_mcts_rollout_per_batch,
      int second_mcts_rollout_per_thread,
      int64_t model_ver);
  void restart();
  void setAsync();
  Coord mcts_make_diverse_move(MCTSGameAI* mcts_ai, Coord c);
  Coord mcts_update_info(MCTSGameAI* mcts_ai, Coord c);
  void finish_game();

 private:
  ThreadedDispatcher* dispatcher_ = nullptr;
  GameNotifierBase* gameNotifier_ = nullptr;
  GameStateExt game_state_ext_;

  int online_counter_ = 0;
  std::unique_ptr<MCTSGameAI> ai1_;
  // Opponent ai (used for selfplay evaluation)
  std::unique_ptr<MCTSGameAI> ai2_;

  std::unique_ptr<SimpleAgent> simple_agent_;
  int ver_ = -1;
  bool swap_ = true;
  int model_update_ = 0;
  
  std::unique_ptr<AIClientT> human_player_;

  std::shared_ptr<spdlog::logger> logger_;
};
