/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <fstream>
#include <iostream>
#include <map>
#include <random>
#include <set>
#include "CheckersState.h"
#include "CheckersFeature.h"
// #include "game_utils.h"
// Game options
#include "../common/go_game_specific.h"
// #include "CheckersGameSpecific.h"
#include "../common/record.h"
#include "Record.h"

#include "elf/ai/tree_search/tree_search_base.h"
#include "elf/logging/IndexedLoggerFactory.h"

enum CheckersFinishReason {
  CHECKERS_MAX_STEP = 0,
  CHEKCERS_BLACK_WIN,
  CHEKCERS_WHITE_WIN,
  // FR_CHEAT_NEWER_WINS_HALF,
  // FR_CHEAT_SELFPLAY_RANDOM_RESULT,
};










struct CheckersStateExt {
 public:
  CheckersStateExt(int game_idx, const GameOptions& options)
      : _game_idx(game_idx),
        _last_move_for_the_game(M_INVALID),
        _last_value(0.0),
        // ,
        // _options(options),
        _logger(
            elf::logging::getLogger("elfgames::go::common::CheckersStateExt-", "")) 
        {
    std::cout << options.info() << std::endl;
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    restart();
  }

  // std::string dumpSgf(const std::string& filename) const;



  // void   dumpSgf() const {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   std::string filename = _options.dump_record_prefix + "_" +
  //       std::to_string(_game_idx) + "_" + std::to_string(_seq) + "_" +
  //       (_state.getFinalValue() > 0 ? "B" : "W") + ".sgf";
  //   std::string sgf_record = dumpSgf(filename);
  //   std::ofstream oo(filename);
  //   oo << sgf_record << std::endl;
  // }



  // void   setRequest(const MsgRequest& request) {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   curr_request_ = request;

  //   const auto& ctrl = request.client_ctrl;
  // }



  // void   addCurrentModel() {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   if (curr_request_.vers.black_ver >= 0)
  //     using_models_.insert(curr_request_.vers.black_ver);
  //   if (curr_request_.vers.white_ver >= 0)
  //     using_models_.insert(curr_request_.vers.white_ver);
  // }



  // const  MsgRequest& currRequest() const {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   return curr_request_;
  // }



  float  setFinalValue(CheckersFinishReason reason, std::mt19937* rng) {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    float final_value = 0.0;

    final_value = _state.evaluate();
    _state.setFinalValue(final_value);
    return final_value;
  }



  Coord  lastMove() const {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    if (_state.justStarted())
      return _last_move_for_the_game;
    else
      return _state.lastMove();
  }



  void   restart() {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    _last_value = _state.getFinalValue();
    _state.reset();
    // _mcts_policies.clear();
    // _predicted_values.clear();

    // using_models_.clear();

    _seq++;

    // addCurrentModel();
  }



  CheckersRecord dumpRecord() const {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    CheckersRecord r;

    // r.timestamp = elf_utils::sec_since_epoch_from_now();
    // r.thread_id = _game_idx;
    // r.seq = _seq;
    // r.request = curr_request_;

    // r.result.reward = _state.getFinalValue();
    // r.result.content = coords2sgfstr(_state.getAllMoves());
    // r.result.using_models =
    //     std::vector<int64_t>(using_models_.begin(), using_models_.end());
    // r.result.policies = _mcts_policies;
    // r.result.num_move = _state.getPly() - 1;
    // r.result.values = _predicted_values;

    return r;
  }



  // ThreadState getThreadState() const {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   ThreadState s;
  //   s.thread_id = _game_idx;
  //   s.seq = _seq;
  //   s.move_idx = _state.getPly() - 1;
  //   s.black = curr_request_.vers.black_ver;
  //   s.white = curr_request_.vers.white_ver;
  //   return s;
  // }



  // void   saveCurrentTree(const std::string& tree_info) const {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   // Dump the tree as well.
  //   std::string filename = _options.dump_record_prefix + "_" +
  //       std::to_string(_game_idx) + "_" + std::to_string(_seq) + "_" +
  //       std::to_string(_state.getPly()) + ".tree";
  //   std::ofstream oo(filename);
  //   oo << _state.showBoard() << std::endl;
  //   oo << tree_info;
  // }



  float  getLastGameFinalValue() const {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    return _last_value;
  }



  // void   addMCTSPolicy(
  //     const elf::ai::tree_search::MCTSPolicy<Coord>& mcts_policy) {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   const auto& policy = mcts_policy.policy;

  //   // First find the max value
  //   float max_val = 0.0;
  //   for (size_t k = 0; k < policy.size(); k++) {
  //     const auto& entry = policy[k];
  //     max_val = std::max(max_val, entry.second);
  //   }

  //   _mcts_policies.emplace_back();
  //   std::fill(
  //       _mcts_policies.back().prob,
  //       _mcts_policies.back().prob + BOUND_COORD,
  //       0);
  //   for (size_t k = 0; k < policy.size(); k++) {
  //     const auto& entry = policy[k];
  //     unsigned char c =
  //         static_cast<unsigned char>(entry.second / max_val * 255);
  //     _mcts_policies.back().prob[entry.first] = c;
  //   }
  // }



  // void   addPredictedValue(float predicted_value) {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   _predicted_values.push_back(predicted_value);
  // }



  // float  getLastPredictedValue() const {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   if (_predicted_values.empty())
  //     return 0.0;
  //   else
  //     return _predicted_values.back();
  // }



  void   showFinishInfo(CheckersFinishReason reason) const;


  bool   forward(Coord c) {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    return _state.forward(c);
  }

  int    seq() const {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    return _seq;
  }

  // bool   finished() const {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   return _options.num_games_per_thread > 0 &&
  //       _seq >= _options.num_games_per_thread;
  // }

  // const  GameOptions& options() const {
  //   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

  //   return _options;
  // }


  const  CheckersState& state() const {
    display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[2;30;43m");

    return _state;
  }

 protected:
  const int 	    _game_idx;
  int			        _seq = 0;

  CheckersState	  _state;
  int 			      _last_move_for_the_game;

  // MsgRequest curr_request_;
  // std::set<int64_t> using_models_;

  float _last_value;
  
  // GameOptions _options;

  // std::vector<CoordRecord> _mcts_policies;
  // std::vector<float> _predicted_values;

  std::shared_ptr<spdlog::logger> _logger;
};



















class CheckersStateExtOffline {
 public:
  friend class GoFeature;

  CheckersStateExtOffline(int game_idx)
  // , const GameOptions& options)
      : _game_idx(game_idx),
        _bf(_state),
        // _options(options),
        _logger(elf::logging::getLogger(
            "elfgames::go::common::CheckersStateExtOffline-",
            "")) 
        {
    display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[2;30;43m");
  }

  // void   fromRecord(const Record& r) {
  //   display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[2;30;43m");

  //   _offline_all_moves = sgfstr2coords(r.result.content);
  //   _offline_winner = r.result.reward > 0 ? 1.0 : -1.0;

  //   _mcts_policies = r.result.policies;
  //   curr_request_ = r.request;
  //   _seq = r.seq;
  //   _predicted_values = r.result.values;
  //   _state.reset();
  // }

  // bool   switchRandomMove(std::mt19937* rng) {
  //   display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[2;30;43m");

  //   // Random sample one move
  //   if ((int)_offline_all_moves.size() <= _options.num_future_actions - 1) {
  //     _logger->warn(
  //         "[{}] #moves {} smaller than {} - 1",
  //         _game_idx,
  //         _offline_all_moves.size(),
  //         _options.num_future_actions);
  //     return false;
  //   }
  //   size_t move_to = (*rng)() %
  //       (_offline_all_moves.size() - _options.num_future_actions + 1);
  //   switchBeforeMove(move_to);
  //   return true;
  // }

  // void   generateD4Code(std::mt19937* rng) {
  //   display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[2;30;43m");
  // }

  // void   switchBeforeMove(size_t move_to) {
  //   display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[2;30;43m");

  //   assert(move_to < _offline_all_moves.size());

  //   _state.reset();
  //   for (size_t i = 0; i < move_to; ++i) {
  //     _state.forward(_offline_all_moves[i]);
  //   }
  // }

  // int  getNumMoves() const {
  //   display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[2;30;43m");

  //   return _offline_all_moves.size();
  // }

  // float getPredictedValue(int move_idx) const {
  //   display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[2;30;43m");
    
  //   return _predicted_values[move_idx];
  // }

 private:
  const int 		_game_idx;
  CheckersState 	_state;
  CheckersFeature	_bf;
  // GameOptions 		_options;

  int _seq;
  // MsgRequest 		curr_request_;

  // std::vector<Coord> _offline_all_moves;
  float _offline_winner;

  // std::vector<CoordRecord> 	_mcts_policies;
  // std::vector<float> 		_predicted_values;

  std::shared_ptr<spdlog::logger> _logger;
};
