/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "game_selfplay.h"
#include "../mcts/ai.h"
#include "../mcts/mcts.h"
#include "go_game_specific.h"

////////////////// GoGame /////////////////////
GoGameSelfPlay::GoGameSelfPlay(
    int game_idx,
    elf::GameClient* client,
    const ContextOptions& context_options,
    const GameOptions& options,
    ThreadedDispatcher* dispatcher,
    GameNotifierBase* notifier,
    CheckersGameNotifierBase* checkers_notifier)
    : GoGameBase(game_idx, client, context_options, options),
      dispatcher_(dispatcher),
      notifier_(notifier),
      checkers_notifier_(checkers_notifier),
      _state_ext(game_idx, options),
      // My
      _checkers_state_ext(game_idx, options),
      logger_(elf::logging::getLogger(
          "elfgames::go::GoGameSelfPlay-" + std::to_string(game_idx) + "-",
          "")) {
    display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);
  }





MCTSGoAI* GoGameSelfPlay::init_ai(
    const std::string& actor_name,
    const elf::ai::tree_search::TSOptions& mcts_options,
    float puct_override,
    int mcts_rollout_per_batch_override,
    int mcts_rollout_per_thread_override,
    int64_t model_ver) {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

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
  params.ply_pass_enabled = _options.ply_pass_enabled;
  params.komi = _options.komi;
  params.required_version = model_ver;

  elf::ai::tree_search::TSOptions opt = mcts_options;
  if (puct_override > 0.0) {
    logger_->warn(
        "PUCT overridden: {} -> {}", opt.alg_opt.c_puct, puct_override);
    opt.alg_opt.c_puct = puct_override;
  }
  if (mcts_rollout_per_batch_override > 0) {
    logger_->warn(
        "Batch size overridden: {} -> {}",
        opt.num_rollouts_per_batch,
        mcts_rollout_per_batch_override);
    opt.num_rollouts_per_batch = mcts_rollout_per_batch_override;
  }
  if (mcts_rollout_per_thread_override > 0) {
    logger_->warn(
        "Rollouts per thread overridden: {} -> {}",
        opt.num_rollouts_per_thread,
        mcts_rollout_per_thread_override);
    opt.num_rollouts_per_thread = mcts_rollout_per_thread_override;
  }
  if (opt.verbose) {
    opt.log_prefix = "ts-game" + std::to_string(_game_idx) + "-mcts";
    logger_->warn("Log prefix {}", opt.log_prefix);
  }

  return new MCTSGoAI(opt, [&](int) { return new MCTSActor(client_, params); });
}






Coord GoGameSelfPlay::mcts_make_diverse_move(MCTSGoAI* mcts_go_ai, Coord c) {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

  auto policy = mcts_go_ai->getMCTSPolicy();

  bool diverse_policy =
      _state_ext.state().getPly() <= _options.policy_distri_cutoff;
  if (diverse_policy) {
    // Sample from the policy.
    c = policy.sampleAction(&_rng);
  }
  if (_options.policy_distri_training_for_all || diverse_policy) {
    // [TODO]: Warning: MCTS Policy might not correspond to move idx.
    _state_ext.addMCTSPolicy(policy);
  }

  return c;
}






Coord GoGameSelfPlay::mcts_update_info(MCTSGoAI* mcts_go_ai, Coord c) {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

  float predicted_value = mcts_go_ai->getValue();

  _state_ext.addPredictedValue(predicted_value);

  if (!_options.dump_record_prefix.empty()) {
    _state_ext.saveCurrentTree(mcts_go_ai->getCurrentTree());
  }

  bool we_are_good = _state_ext.state().nextPlayer() == S_BLACK
      ? ((getScore() > 0) && (predicted_value > 0.9))
      : ((getScore() < 0) && (predicted_value < -0.9));
  // If the opponent wants pass, and we are in good, we follow.
  if (_human_player != nullptr && we_are_good &&
      _state_ext.state().lastMove() == M_PASS && _options.following_pass)
    c = M_PASS;

  // Check the ranking of selected move.
  if (notifier_ != nullptr) {
    notifier_->OnMCTSResult(c, mcts_go_ai->getLastResult());
  }
  return c;
}






void GoGameSelfPlay::finish_game(FinishReason reason) {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

  // посмотреть зачем нужны эти опции тк это опции игры
  if (!_state_ext.currRequest().vers.is_selfplay() &&
      _options.cheat_eval_new_model_wins_half) {
    reason = FR_CHEAT_NEWER_WINS_HALF;
  }
  if (_state_ext.currRequest().vers.is_selfplay() &&
      _options.cheat_selfplay_random_result) {
    reason = FR_CHEAT_SELFPLAY_RANDOM_RESULT;
  }

  _state_ext.setFinalValue(reason, &_rng);
  _state_ext.showFinishInfo(reason);

  if (!_options.dump_record_prefix.empty()) {
    _state_ext.dumpSgf();
  }

  // reset tree if MCTS_AI, otherwise just do nothing
  go_ai1->endGame(_state_ext.state());
  if (go_ai2 != nullptr) {
    go_ai2->endGame(_state_ext.state());
  }

  if (notifier_ != nullptr) {
    notifier_->OnGameEnd(_state_ext);
  }

  // clear state, MCTS polices et.al.
  _state_ext.restart();
}


void GoGameSelfPlay::finish_game(CheckersFinishReason reason) {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

  // My code
  _checkers_state_ext.setFinalValue(reason, &_rng);
  _checkers_state_ext.showFinishInfo(reason);

  // if (!_options.dump_record_prefix.empty()) {
  //   _state_ext.dumpSgf();
  // }

  // reset tree if MCTS_AI, otherwise just do nothing
  // go_ai1->endGame(_state_ext.state());
  // if (go_ai2 != nullptr) {
  //   go_ai2->endGame(_state_ext.state());
  // }

  // сообщает клиенту, что игры окончена
  if (checkers_notifier_ != nullptr){
    std::cout << "checkers_notifier_ != nullptr" << std::endl;
    checkers_notifier_->OnGameEnd(_checkers_state_ext);
  }

  // My code
  _checkers_state_ext.restart();  

  // exit(1);
}





void GoGameSelfPlay::setAsync() {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

  go_ai1->setRequiredVersion(-1);
  if (go_ai2 != nullptr)
    go_ai2->setRequiredVersion(-1);

  _state_ext.addCurrentModel();
}






void GoGameSelfPlay::restart() {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

  const MsgRequest& request = _state_ext.currRequest();
  bool async = request.client_ctrl.async;

  go_ai1.reset(nullptr);
  go_ai2.reset(nullptr);
  if (_options.mode == "selfplay") {
    go_ai1.reset(init_ai(
        "actor_black",
        request.vers.mcts_opt,
        -1.0,
        -1,
        -1,
        async ? -1 : request.vers.black_ver));
    if (request.vers.white_ver >= 0) {
      go_ai2.reset(init_ai(
          "actor_white",
          request.vers.mcts_opt,
          _state_ext.options().white_puct,
          _state_ext.options().white_mcts_rollout_per_batch,
          _state_ext.options().white_mcts_rollout_per_thread,
          async ? -1 : request.vers.white_ver));
    }
    if (!request.vers.is_selfplay() && request.client_ctrl.player_swap) {
      // Swap the two pointer.
      swap(go_ai1, go_ai2);
    }
  } else if (_options.mode == "online") {
    go_ai1.reset(init_ai(
        "actor_black",
        request.vers.mcts_opt,
        -1.0,
        -1,
        -1,
        request.vers.black_ver));
    _human_player.reset(new AI(client_, {"human_actor"}));
  } else {
    logger_->critical("Unknown mode! {}", _options.mode);
    throw std::range_error("Unknown mode");
  }


  _state_ext.restart();
}






bool GoGameSelfPlay::OnReceive(const MsgRequest& request, RestartReply* reply) {
  // при связи с сервером
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);

  if (*reply == RestartReply::UPDATE_COMPLETE)
    return false;

  bool is_waiting = request.vers.wait();
  bool is_prev_waiting = _state_ext.currRequest().vers.wait();

  if (_options.verbose && !(is_waiting && is_prev_waiting)) {
    logger_->debug(
        "Receive request: {}, old: {}",
        (!is_waiting ? request.info() : "[wait]"),
        (!is_prev_waiting ? _state_ext.currRequest().info() : "[wait]"));
  }

  bool same_vers = (request.vers == _state_ext.currRequest().vers);
  bool same_player_swap =
      (request.client_ctrl.player_swap ==
       _state_ext.currRequest().client_ctrl.player_swap);

  bool async = request.client_ctrl.async;

  bool no_restart =
      (same_vers || async) && same_player_swap && !is_prev_waiting;

  // Then we need to reset everything.
  _state_ext.setRequest(request);

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
























void GoGameSelfPlay::act() {
  display_debug_info("GoGameSelfPlay", __FUNCTION__, RED_B);
  
  if (_online_counter % 5 == 0) {
    using std::placeholders::_1;
    using std::placeholders::_2;
    auto f = std::bind(&GoGameSelfPlay::OnReceive, this, _1, _2);

    do {
      dispatcher_->checkMessage(_state_ext.currRequest().vers.wait(), f);
    } while (_state_ext.currRequest().vers.wait());

    // Check request every 5 times.
    // Update current state.
    if (notifier_ != nullptr) {
      notifier_->OnStateUpdate(_state_ext.getThreadState());
    }
  }
  _online_counter++;









  // bool show_board = (_options.verbose && _context_options.num_games == 1);
  // const GoState& s = _state_ext.state();

  // if (_human_player != nullptr) {
  //   do {
  //     if (s.terminated()) {
  //       finish_game(FR_ILLEGAL);
  //       return;
  //     }
  //     // if (сs.terminated())
  //     //   finish_game(FR_ILLEGAL);

  //     BoardFeature bf(s);
  //     GoReply reply(bf);
  //     _human_player->act(bf, &reply);

  //     // Checkers
  //     // CheckersFeature cf(cs);
  //     // CheckersReply creply(cf);
  //     // _human_plyaer->act(cf, &creply);

  //     // skip the current move, and ask the ai to move.
  //     if (reply.c == M_SKIP)
  //       break;
  //     if (reply.c == M_CLEAR) {
  //       if (!_state_ext.state().justStarted()) {
  //         finish_game(FR_CLEAR);
  //       }
  //       return;
  //     }
  //     // Otherwise we forward.
  //     if (_state_ext.forward(reply.c)) {
  //       if (_state_ext.state().isTwoPass()) {
  //         // If the human opponent pass, we pass as well.
  //         finish_game(FR_TWO_PASSES);
  //       }
  //       return;
  //     }
  //     logger_->warn(
  //         "Invalid move: x = {} y = {} move: {} please try again",
  //         X(reply.c),
  //         Y(reply.c),
  //         coord2str(reply.c));
  //   } while (!client_->checkPrepareToStop());
  // } else {
  //   // If re receive this, then we should not send games anymore
  //   // (otherwise the process never stops)
  //   if (client_->checkPrepareToStop()) {
  //     // [TODO] A lot of hack here. We need to fix it later.
  //     AI ai(client_, {"actor_black"});
  //     BoardFeature bf(s);
  //     GoReply reply(bf);
  //     ai.act(bf, &reply);

  //     if (client_->DoStopGames())
  //       return;

  //     AI ai_white(client_, {"actor_white"});
  //     ai_white.act(bf, &reply);

  //     elf::FuncsWithState funcs = client_->BindStateToFunctions(
  //         {"game_start"}, &_state_ext.currRequest().vers);
  //     client_->sendWait({"game_start"}, &funcs);

  //     funcs = client_->BindStateToFunctions({"game_end"}, &_state_ext.state());
  //     client_->sendWait({"game_end"}, &funcs);

  //     logger_->info("Received command to prepare to stop");
  //     std::this_thread::sleep_for(std::chrono::seconds(1));
  //     return;
  //   }
  // }

  // Stone player = s.nextPlayer();
  // bool use_policy_network_only =
  //     (player == S_WHITE && _options.white_use_policy_network_only) ||
  //     (player == S_BLACK && _options.black_use_policy_network_only);

  // printf("use_policy_network_only : %d\n", use_policy_network_only);
  
  // // Coord c = M_INVALID;
  // MCTSGoAI* curr_ai =
  //     ((go_ai2 != nullptr && player == S_WHITE) ? go_ai2.get() : go_ai1.get());

  // if (use_policy_network_only) {
  //   // Then we only use policy network to move.
  //   curr_ai->actPolicyOnly(s, &c);
  // } else {

  //   std::cout << "curr_ai->act(s, &c);" << std::endl;
    
  //   curr_ai->act(s, &c);
  //   c = mcts_make_diverse_move(curr_ai, c);
  // }

  // c = mcts_update_info(curr_ai, c);

  // if (show_board) {
  //   logger_->info(
  //       "Current board:\n{}\n[{}] Propose move {}\n",
  //       s.showBoard(),
  //       s.getPly(),
  //       elf::ai::tree_search::ActionTrait<Coord>::to_string(c));
  // }
  // if (!_state_ext.forward(c)) {
  //   logger_->error(
  //       "Something is wrong! Move {} cannot be applied\nCurrent board: "
  //       "{}\n[{}] Propose move {}\nSGF: {}\n",
  //       c,
  //       s.showBoard(),
  //       s.getPly(),
  //       elf::ai::tree_search::ActionTrait<Coord>::to_string(c),
  //       _state_ext.dumpSgf(""));
  //   return;
  // }

  // if (s.terminated()) {
  //   auto reason = s.isTwoPass()
  //       ? FR_TWO_PASSES
  //       : s.getPly() >= BOARD_MAX_MOVE ? FR_MAX_STEP : FR_ILLEGAL;
  //   finish_game(reason);
  // }

  // if (_options.move_cutoff > 0 && s.getPly() >= _options.move_cutoff) {
  //   finish_game(FR_MAX_STEP);
  // }

  // ===================================================
  // ===================================================
  // ===================================================
  // checkers
  // Checkers
  const CheckersState& cs = _checkers_state_ext.state();





// elf::ai::AIClientT






if (client_->checkPrepareToStop()) {
    // [TODO] A lot of hack here. We need to fix it later.
    CheckersFeature cf(cs);
    CheckersReply   creply(cf);
    
    CheckersAI ai_black(client_, {"checkers_actor_black"});
    ai_black.act(cf, &creply);

    if (client_->DoStopGames())
      return;

    CheckersAI ai_white(client_, {"checkers_actor_white"});
    ai_white.act(cf, &creply);

    elf::FuncsWithState funcs = client_->BindStateToFunctions(
        {"game_start"}, &_state_ext.currRequest().vers);
    client_->sendWait({"game_start"}, &funcs);

    funcs = client_->BindStateToFunctions({"game_end"}, &_state_ext.state());
    client_->sendWait({"game_end"}, &funcs);

    logger_->info("Received command to prepare to stop");
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return;
  }









  int current_player = cs.nextPlayer();
  int move = M_INVALID;

  CheckersFeature cf(cs);
  CheckersReply creply(cf);

  std::string player = current_player == BLACK_PLAYER ? "checkers_actor_black" 
                                                      : "checkers_actor_black";

  // MCTSGoAI* ai = checkers_ai.get();
  // AIClientT checkers_ai(client_, "checkers_actor_white");
  // checkers_ai->act(cf, &creply);

  elf::FuncsWithState funcs_s = client_->BindStateToFunctions({player}, &cf);
  elf::FuncsWithState funcs_a = client_->BindStateToFunctions({player}, &creply);
  funcs_s.add(funcs_a);

  comm::ReplyStatus status = client_->sendWait({player}, &funcs_s);

  // std::cout << "player : " << player << std::endl;
  // creply.info();
  // std::cout << cs.showBoard() << std::endl;



  // пытаемся делать шаг
  // доостаем валидные шаги
  std::vector<float> pi = creply.pi;
  CheckersState c_state = this->_checkers_state_ext.state();
  CheckersBoard c_board = c_state.board();
  std::array<int, ALL_ACTIONS> moves = GetValidMovesBinary(c_board, current_player);
  float temp_pi = 0.0;
  for (int i = 0; i < moves.size(); i++){
    if (moves[i] != 0 && pi[i] > temp_pi){
      temp_pi = pi[i];
      move = i;
    }
  }

  // берем лучший action по вероятностям

  if (!_checkers_state_ext.forward(move)) {
    logger_->error(
        "Something is wrong! Move {} cannot be applied\nCurrent board: "
        "{}\n[{}] Propose move {}\nSGF: {}\n",
        move,
        cs.showBoard(),
        cs.getPly()
        // elf::ai::tree_search::ActionTrait<Coord>::to_string(c),
        // _state_ext.dumpSgf("")
        );    
    return;
  }
  
  if (cs.terminated()) {
    CheckersFinishReason reason = cs.getPly() >= BOARD_MAX_MOVE ? CHECKERS_MAX_STEP : 
    (cs.nextPlayer() == WHITE_PLAYER) ? CHEKCERS_BLACK_WIN : CHEKCERS_WHITE_WIN;
    finish_game(reason);
  }

  if (_options.move_cutoff > 0 && cs.getPly() >= _options.move_cutoff) {
    CheckersFinishReason reason = CHECKERS_MAX_STEP;
    finish_game(reason);
  }

  std::cout << cs.showBoard() << std::endl;
  // 
  // exit(1);
}
























