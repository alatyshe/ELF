/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ClientGameSelfPlay.h"

////////////////// Game /////////////////////
ClientGameSelfPlay::ClientGameSelfPlay(
    int game_idx,
    elf::GameClient* client,
    const ContextOptions& context_options,
    const GameOptions& game_options,
    ThreadedDispatcher* dispatcher,
    GameNotifierBase* gameNotifier)
    : GameBase(game_idx, client, context_options, game_options),
      dispatcher_(dispatcher),
      gameNotifier_(gameNotifier),
      game_state_ext_(game_idx, game_options),
      logger_(elf::logging::getIndexedLogger(
          MAGENTA_B + std::string("|++|") + COLOR_END + 
          "ClientGameSelfPlay-" + std::to_string(game_idx) + "-",
          "")) {
}

std::string ClientGameSelfPlay::showBoard() const {
  return game_state_ext_.state().showBoard();
}

std::array<int, TOTAL_NUM_ACTIONS> ClientGameSelfPlay::getValidMoves() const {
  return GetValidMovesBinary(game_state_ext_.state().board());
}

float ClientGameSelfPlay::getScore() {
  return game_state_ext_.state().evaluateGame();
}

MCTSGameAI* ClientGameSelfPlay::init_ai(
    const std::string& actor_name,
    const elf::ai::tree_search::TSOptions& mcts_options,
    float puct_override,
    int mcts_rollout_per_batch_override,
    int mcts_rollout_per_thread_override,
    int64_t model_ver) {
  logger_->info(
      "Initializing actor {}; puct_override: {}; batch_override: {}; "
      "per_thread_override: {}",
      actor_name,
      puct_override,
      mcts_rollout_per_batch_override,
      mcts_rollout_per_thread_override);

  MCTSActorParams params;
  
  params.actor_name = actor_name;
  params.seed = _rng();
  params.required_version = model_ver;

  elf::ai::tree_search::TSOptions mcts_opt = mcts_options;
  // My
  // mcts_opt.verbose = true;

  if (puct_override > 0.0) {
    logger_->warn(
        "PUCT overridden: {} -> {}", mcts_opt.alg_opt.c_puct, puct_override);
    mcts_opt.alg_opt.c_puct = puct_override;
  }
  if (mcts_rollout_per_batch_override > 0) {
    logger_->warn(
        "Batch size overridden: {} -> {}",
        mcts_opt.num_rollouts_per_batch,
        mcts_rollout_per_batch_override);
    mcts_opt.num_rollouts_per_batch = mcts_rollout_per_batch_override;
  }
  if (mcts_rollout_per_thread_override > 0) {
    logger_->warn(
        "Rollouts per thread overridden: {} -> {}",
        mcts_opt.num_rollouts_per_thread,
        mcts_rollout_per_thread_override);
    mcts_opt.num_rollouts_per_thread = mcts_rollout_per_thread_override;
  }

  if (mcts_opt.verbose) {
    mcts_opt.log_prefix = "ts-game" + std::to_string(_game_idx) + "-mcts";
    logger_->warn("Log prefix {}", mcts_opt.log_prefix);
  }

  return new MCTSGameAI(mcts_opt, [&](int) { return new MCTSGameActor(client_, params); });
}

Coord ClientGameSelfPlay::mcts_make_diverse_move(MCTSGameAI* mcts_ai, Coord c) {
  auto policy = mcts_ai->getMCTSPolicy();

  // make random move if diverse_policy == true
  bool diverse_policy =
      game_state_ext_.state().getPly() <= _game_options.policy_distri_cutoff;
  if (diverse_policy) {
    // Sample from the policy.
    c = policy.sampleAction(&_rng);
  }

  if (_game_options.policy_distri_training_for_all || diverse_policy) {
    // [TODO]: Warning: MCTS Policy might not correspond to move idx.
    game_state_ext_.addMCTSPolicy(policy);
  }

  return c;
}

Coord ClientGameSelfPlay::mcts_update_info(MCTSGameAI* mcts_ai, Coord c) {
  float predicted_value = mcts_ai->getValue();

  game_state_ext_.addPredictedValue(predicted_value);

  if (!_game_options.dump_record_prefix.empty()) {
    game_state_ext_.saveCurrentTree(mcts_ai->getCurrentTree());
  }
  return c;
}

void ClientGameSelfPlay::finish_game() {
  swap_ = swap_ ? false : true;

  // My code
  game_state_ext_.setFinalValue();
  // show board
  game_state_ext_.showFinishInfo();

  // if (!_game_options.dump_record_prefix.empty()) {
  //   _state_ext.dumpSgf();
  // }

  // reset tree if MCTS_AI, otherwise just do nothing
  ai1_->endGame(game_state_ext_.state());
  if (ai2_ != nullptr) {
    ai2_->endGame(game_state_ext_.state());
  }
  // Says python that game is over.
  if (gameNotifier_ != nullptr){
    gameNotifier_->OnGameEnd(game_state_ext_);
  }
  // My code
  game_state_ext_.restart();
}

void ClientGameSelfPlay::setAsync() {
  ai1_->setRequiredVersion(-1);
  if (ai2_ != nullptr)
    ai2_->setRequiredVersion(-1);

  game_state_ext_.addCurrentModel();
}

void ClientGameSelfPlay::restart() {
  const MsgRequest& request = game_state_ext_.currRequest();
  bool async = request.client_ctrl.async;

  ai1_.reset(nullptr);
  ai2_.reset(nullptr);

  simple_agent_.reset(new SimpleAgent());

  if (_game_options.mode == "selfplay") {
    ai1_.reset(init_ai(
        "actor_black",
        request.vers.mcts_opt,
        -1.0,
        -1,
        -1,
        async ? -1 : request.vers.black_ver));
    if (request.vers.white_ver >= 0) {
      ai2_.reset(init_ai(
          "actor_white",
          request.vers.mcts_opt,
          game_state_ext_.gameOptions().white_puct,
          game_state_ext_.gameOptions().white_mcts_rollout_per_batch,
          game_state_ext_.gameOptions().white_mcts_rollout_per_thread,
          async ? -1 : request.vers.white_ver));
    }
    if (!request.vers.is_selfplay() && request.client_ctrl.player_swap) {
      // Swap the two pointer.
      swap(ai1_, ai2_);
    }
  } else if (_game_options.mode == "play") {
    ai1_.reset(init_ai(
        "actor_black",
        request.vers.mcts_opt,
        -1.0,
        -1,
        -1,
        request.vers.black_ver));
    human_player_.reset(new AIClientT(client_, {"human_actor"}));
  } else {
    logger_->critical("Unknown mode! {}", _game_options.mode);
    throw std::range_error("Unknown mode");
  }
  game_state_ext_.restart();
}

bool ClientGameSelfPlay::OnReceive(const MsgRequest& request, RestartReply* reply) {
  // при связи с сервером
  if (*reply == RestartReply::UPDATE_COMPLETE)
    return false;


  bool is_waiting = request.vers.wait();
  bool is_prev_waiting = game_state_ext_.currRequest().vers.wait();

  if (_game_options.verbose && !(is_waiting && is_prev_waiting)) {
    logger_->debug(
        "Receive request: {}, old: {}",
        (!is_waiting ? request.info() : "[wait]"),
        (!is_prev_waiting ? game_state_ext_.currRequest().info() : "[wait]"));
  }

  bool same_vers = (request.vers == game_state_ext_.currRequest().vers);
  bool same_player_swap =
      (request.client_ctrl.player_swap ==
       game_state_ext_.currRequest().client_ctrl.player_swap);

  bool async = request.client_ctrl.async;

  bool no_restart =
      (same_vers || async) && same_player_swap && !is_prev_waiting;

  // Then we need to reset everything.
  game_state_ext_.setRequest(request);

  if (is_waiting) {
    *reply = RestartReply::ONLY_WAIT;
    return false;
  } else {
    model_update_++;

    if (!no_restart) {
      restart();
      *reply = RestartReply::UPDATE_MODEL;
      return true;
    } else {
      if (!async)
        *reply = RestartReply::UPDATE_REQUEST_ONLY;
      else {
        setAsync();
        if (same_vers)
          *reply = RestartReply::UPDATE_REQUEST_ONLY;
        else
          *reply = RestartReply::UPDATE_MODEL_ASYNC;
      }
      return false;
    }
  }
}

void ClientGameSelfPlay::act() {
  if (online_counter_ % 5 == 0) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    auto f = std::bind(&ClientGameSelfPlay::OnReceive, this, _1, _2);

    do {
      dispatcher_->checkMessage(game_state_ext_.currRequest().vers.wait(), f);
    } while (game_state_ext_.currRequest().vers.wait());

    // Check request every 5 times.
    // Update current state.
    if (gameNotifier_ != nullptr) {
      gameNotifier_->OnStateUpdate(game_state_ext_.getThreadState());
    }
  }
  online_counter_++;


  const GameState& gameState = game_state_ext_.state();
  // just display board on every move
  if (human_player_ != nullptr)
    std::cout << gameState.showBoard() << std::endl;


  if (human_player_ != nullptr 
        && gameState.currentPlayer() == _game_options.human_plays_for) {
    do {
      BoardFeature    boardFeature(gameState);
      BoardReply      boardReply(boardFeature);
      human_player_->act(boardFeature, &boardReply);

      if (boardReply.c == -1) {
        finish_game();
        return;
      } else if (boardReply.c == -2) {
        if (_game_options.human_plays_for == BLACK_PLAYER)
          _game_options.human_plays_for = WHITE_PLAYER;
        else
          _game_options.human_plays_for = BLACK_PLAYER;
        finish_game();
        return;
      }

      if (game_state_ext_.forward(boardReply.c)) {
        if (gameState.terminated()) {
          finish_game();
        }
        return;
      }
      logger_->warn(
          "Invalid move move: {} please try again",
          boardReply.c);
    } while (!client_->checkPrepareToStop());
  } else {

    if (client_->checkPrepareToStop()) {
      // [TODO] A lot of hack here. We need to fix it later.
      BoardFeature  boardFeature(gameState);
      BoardReply     boardReply(boardFeature);
      
      AIClientT ai_black(client_, {"actor_black"});
      ai_black.act(boardFeature, &boardReply);

      if (client_->DoStopGames())
        return;

      AIClientT ai_white(client_, {"actor_white"});
      ai_white.act(boardFeature, &boardReply);

      elf::FuncsWithState funcs = client_->BindStateToFunctions(
          {"game_start"}, &game_state_ext_.currRequest().vers);
      client_->sendWait({"game_start"}, &funcs);

      funcs = client_->BindStateToFunctions({"game_end"}, &game_state_ext_.state());
      client_->sendWait({"game_end"}, &funcs);

      logger_->info("Received command to prepare to stop");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }
  }

  int current_player = gameState.currentPlayer();
  Coord move = M_INVALID;

  if (model_update_ == 2
        && ((current_player == BLACK_PLAYER && swap_ == true) 
        ||  (current_player == WHITE_PLAYER && swap_ == false))) {
    SimpleAgent *agent = simple_agent_.get();
    move = agent->GetBestMove(gameState.board());
  } else if (model_update_ == 1) {
    SimpleAgent *agent = simple_agent_.get();
    usleep(100000);
    move = agent->GetBestMove(gameState.board());
  } else {
    bool use_policy_network_only =
        (current_player == WHITE_PLAYER && _game_options.white_use_policy_network_only) ||
        (current_player == BLACK_PLAYER && _game_options.black_use_policy_network_only);

    MCTSGameAI* curr_ai =
      ((ai2_ != nullptr && current_player == WHITE_PLAYER) 
        ? ai2_.get() : ai1_.get());

    // use_policy_network_only = true;
    if (use_policy_network_only) {
      // Then we only use policy network to move.
      curr_ai->actPolicyOnly(gameState, &move);
    } else {
      curr_ai->act(gameState, &move);
      move = mcts_make_diverse_move(curr_ai, move);
    }
    move = mcts_update_info(curr_ai, move);
  }

  // make move
  if (!game_state_ext_.forward(move)) {
    logger_->error(
        "Something is wrong! Move {} cannot be applied\nCurrent board: \n"
        "{}\n[{}] Propose move {}\n",
        move,
        gameState.showBoard(),
        gameState.getPly(),
        (*moves::m_to_h.find(move)).second
        );
    exit(0);
    return;
  }

  if (gameState.terminated()) {
    finish_game();
  }
}

// Gui
std::array<std::array<int, 8>, 8> ClientGameSelfPlay::getBoard() const {
  return game_state_ext_.state().getBoard();
}

int ClientGameSelfPlay::getCurrentPlayer() const {
  return game_state_ext_.state().currentPlayer();
}
