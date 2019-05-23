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
      _game_state_ext(game_idx, game_options),
      logger_(elf::logging::getIndexedLogger(
          MAGENTA_B + std::string("|++|") + COLOR_END + 
          "ClientGameSelfPlay-" + std::to_string(game_idx) + "-",
          "")) {
}

std::string ClientGameSelfPlay::showBoard() const {
  return _game_state_ext.state().showBoard();
}

std::array<int, TOTAL_NUM_ACTIONS> ClientGameSelfPlay::getValidMoves() const {
  return GetValidMovesBinary(_game_state_ext.state().board());
}

float ClientGameSelfPlay::getScore() {
  return _game_state_ext.state().evaluateGame();
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
      _game_state_ext.state().getPly() <= _game_options.policy_distri_cutoff;
  if (diverse_policy) {
    // Sample from the policy.
    c = policy.sampleAction(&_rng);
  }

  if (_game_options.policy_distri_training_for_all || diverse_policy) {
    // [TODO]: Warning: MCTS Policy might not correspond to move idx.
    _game_state_ext.addMCTSPolicy(policy);
  }

  return c;
}

Coord ClientGameSelfPlay::mcts_update_info(MCTSGameAI* mcts_ai, Coord c) {
  float predicted_value = mcts_ai->getValue();

  _game_state_ext.addPredictedValue(predicted_value);

  if (!_game_options.dump_record_prefix.empty()) {
    _game_state_ext.saveCurrentTree(mcts_ai->getCurrentTree());
  }
  return c;
}

void ClientGameSelfPlay::finish_game() {
  // My code
  _game_state_ext.setFinalValue();
  // show board
  _game_state_ext.showFinishInfo();

  // if (!_game_options.dump_record_prefix.empty()) {
  //   _state_ext.dumpSgf();
  // }

  // reset tree if MCTS_AI, otherwise just do nothing
  _ai1->endGame(_game_state_ext.state());
  if (_ai2 != nullptr) {
    _ai2->endGame(_game_state_ext.state());
  }
  // Says python that game is over.
  if (gameNotifier_ != nullptr){
    gameNotifier_->OnGameEnd(_game_state_ext);
  }
  // My code
  _game_state_ext.restart();
}

void ClientGameSelfPlay::setAsync() {
  _ai1->setRequiredVersion(-1);
  if (_ai2 != nullptr)
    _ai2->setRequiredVersion(-1);

  _game_state_ext.addCurrentModel();
}

void ClientGameSelfPlay::restart() {
  const MsgRequest& request = _game_state_ext.currRequest();
  bool async = request.client_ctrl.async;

  _ai1.reset(nullptr);
  _ai2.reset(nullptr);
  if (_game_options.mode == "selfplay") {
    _ai1.reset(init_ai(
        "actor_black",
        request.vers.mcts_opt,
        -1.0,
        -1,
        -1,
        async ? -1 : request.vers.black_ver));
    if (request.vers.white_ver >= 0) {
      _ai2.reset(init_ai(
          "actor_white",
          request.vers.mcts_opt,
          _game_state_ext.gameOptions().white_puct,
          _game_state_ext.gameOptions().white_mcts_rollout_per_batch,
          _game_state_ext.gameOptions().white_mcts_rollout_per_thread,
          async ? -1 : request.vers.white_ver));
    }
    if (!request.vers.is_selfplay() && request.client_ctrl.player_swap) {
      // Swap the two pointer.
      swap(_ai1, _ai2);
    }
  } else if (_game_options.mode == "play") {
    _ai1.reset(init_ai(
        "actor_black",
        request.vers.mcts_opt,
        -1.0,
        -1,
        -1,
        request.vers.black_ver));    
    _human_player.reset(new AIClientT(client_, {"actor_white"}));
  } else {
    logger_->critical("Unknown mode! {}", _game_options.mode);
    throw std::range_error("Unknown mode");
  }
  _game_state_ext.restart();
}

bool ClientGameSelfPlay::OnReceive(const MsgRequest& request, RestartReply* reply) {
  // при связи с сервером
  if (*reply == RestartReply::UPDATE_COMPLETE)
    return false;

  bool is_waiting = request.vers.wait();
  bool is_prev_waiting = _game_state_ext.currRequest().vers.wait();

  if (_game_options.verbose && !(is_waiting && is_prev_waiting)) {
    logger_->debug(
        "Receive request: {}, old: {}",
        (!is_waiting ? request.info() : "[wait]"),
        (!is_prev_waiting ? _game_state_ext.currRequest().info() : "[wait]"));
  }

  bool same_vers = (request.vers == _game_state_ext.currRequest().vers);
  bool same_player_swap =
      (request.client_ctrl.player_swap ==
       _game_state_ext.currRequest().client_ctrl.player_swap);

  bool async = request.client_ctrl.async;

  bool no_restart =
      (same_vers || async) && same_player_swap && !is_prev_waiting;

  // Then we need to reset everything.
  _game_state_ext.setRequest(request);

  if (is_waiting) {
    *reply = RestartReply::ONLY_WAIT;
    return false;
  } else {
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
  if (_online_counter % 5 == 0) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    auto f = std::bind(&ClientGameSelfPlay::OnReceive, this, _1, _2);

    do {
      dispatcher_->checkMessage(_game_state_ext.currRequest().vers.wait(), f);
    } while (_game_state_ext.currRequest().vers.wait());

    // Check request every 5 times.
    // Update current state.
    if (gameNotifier_ != nullptr) {
      gameNotifier_->OnStateUpdate(_game_state_ext.getThreadState());
    }
  }
  _online_counter++;

  const GameState& cs = _game_state_ext.state();
  // just display board on every move
  if (_human_player != nullptr)
    std::cout << cs.showBoard() << std::endl;


  if (_human_player != nullptr 
        && cs.currentPlayer() == _game_options.human_plays_for) {
    do {
      BoardFeature board_feature(cs);
      BoardReply   board_reply(board_feature);
      _human_player->act(board_feature, &board_reply);

      if (board_reply.c == -100) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return;
      } else if (board_reply.c == -1) {
        finish_game();
        return;
      } else if (board_reply.c == -2) {
        if (_game_options.human_plays_for == BLACK_PLAYER)
          _game_options.human_plays_for = WHITE_PLAYER;
        else
          _game_options.human_plays_for = BLACK_PLAYER;
        finish_game();
        return;
      }

      if (_game_state_ext.forward(board_reply.c)) {
        if (cs.terminated()) {
          finish_game();
        }
        return;
      }
      logger_->warn(
          "Invalid move move: {} please try again",
          board_reply.c);
    } while (!client_->checkPrepareToStop());
  } else {

    if (client_->checkPrepareToStop()) {
      // [TODO] A lot of hack here. We need to fix it later.
      BoardFeature board_feature(cs);
      BoardReply   board_reply(board_feature);
      
      AIClientT ai_black(client_, {"actor_black"});
      ai_black.act(board_feature, &board_reply);

      if (client_->DoStopGames())
        return;

      AIClientT ai_white(client_, {"actor_white"});
      ai_white.act(board_feature, &board_reply);

      elf::FuncsWithState funcs = client_->BindStateToFunctions(
          {"game_start"}, &_game_state_ext.currRequest().vers);
      client_->sendWait({"game_start"}, &funcs);

      funcs = client_->BindStateToFunctions({"game_end"}, &_game_state_ext.state());
      client_->sendWait({"game_end"}, &funcs);

      logger_->info("Received command to prepare to stop");
      std::this_thread::sleep_for(std::chrono::seconds(1));
      return;
    }
  }

  int current_player = cs.currentPlayer();
  Coord move = M_INVALID;

  BoardFeature board_feature(cs);
  BoardReply   board_reply(board_feature);

  bool use_policy_network_only =
      (current_player == WHITE_PLAYER && _game_options.white_use_policy_network_only) ||
      (current_player == BLACK_PLAYER && _game_options.black_use_policy_network_only);

  MCTSGameAI* curr_ai =
    ((_ai2 != nullptr && current_player == WHITE_PLAYER) 
      ? _ai2.get() : _ai1.get());
  
  // use_policy_network_only = true;
  if (use_policy_network_only) {
    // Then we only use policy network to move.
    curr_ai->actPolicyOnly(cs, &move);
  } else {
    curr_ai->act(cs, &move);
    move = mcts_make_diverse_move(curr_ai, move);
  }

  move = mcts_update_info(curr_ai, move);

  // make move
  if (!_game_state_ext.forward(move)) {
    logger_->error(
        "Something is wrong! Move {} cannot be applied\nCurrent board: "
        "{}\n[{}] Propose move {}\n",
        move,
        cs.showBoard(),
        cs.getPly(),
        std::to_string(move)
        );
    return;
  }

  if (cs.terminated()) {
    finish_game();
  }
}

// Gui
std::array<std::array<int, 8>, 8> ClientGameSelfPlay::getBoard() const {
  return _game_state_ext.state().getBoard();
}

int ClientGameSelfPlay::getCurrentPlayer() const {
  return _game_state_ext.state().currentPlayer();
}
