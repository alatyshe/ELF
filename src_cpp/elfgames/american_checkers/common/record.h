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
#include <mutex>
#include <string>
#include <vector>
#include <nlohmann/json.hpp>

// game
#include "ModelPair.h"
#include "../sgf/sgf.h"

using json = nlohmann::json;

/*
  SELFPLAY_ONLY - implies that this client will only generate batches.
  CLIENT_EVAL_THEN_SELFPLAY - implies that this client will, 
    in addition to generating games, evaluate the models that 
    the server will send on evaluation.
*/
enum ClientType {
  CLIENT_INVALID,
  CLIENT_SELFPLAY_ONLY,
  CLIENT_EVAL_THEN_SELFPLAY
};


/* 
  Information about client, used in MsgRequest.
*/
struct ClientCtrl {
  ClientType client_type = CLIENT_SELFPLAY_ONLY;
  // -1 means to use all the threads.
  int num_game_thread_used = -1;
  bool player_swap = false;
  bool async = false;

  void setJsonFields(json& j) const {
    JSON_SAVE(j, client_type);
    JSON_SAVE(j, num_game_thread_used);
    JSON_SAVE(j, player_swap);
    JSON_SAVE(j, async);
  }

  static ClientCtrl createFromJson(
      const json& j,
      bool player_swap_optional = false) {
    ClientCtrl ctrl;

    JSON_LOAD(ctrl, j, client_type);
    JSON_LOAD(ctrl, j, num_game_thread_used);
    // For backward compatibility.
    if (player_swap_optional) {
      JSON_LOAD_OPTIONAL(ctrl, j, player_swap);
    } else {
      JSON_LOAD(ctrl, j, player_swap);
    }
    JSON_LOAD_OPTIONAL(ctrl, j, async);
    return ctrl;
  }

  std::string info() const {
    std::stringstream ss;
    ss  << "\nClientCtrl::info()\t[client=";

    if (client_type == CLIENT_INVALID)
      ss << "invalid]";
    else if (client_type == CLIENT_SELFPLAY_ONLY)
      ss << CYAN_B << "CLIENT_SELFPLAY_ONLY" << COLOR_END << "]";
    else if (client_type == CLIENT_EVAL_THEN_SELFPLAY)
      ss << WHITE_B << "CLIENT_EVAL_THEN_SELFPLAY" << COLOR_END << "]";
    else
      ss << client_type << "]";

    ss  << "[Player_swap=" << player_swap << "]"
        << "[async=" << async << "]"
        << "[#th=" << num_game_thread_used << "]";

    return ss.str();
  }

  friend bool operator==(const ClientCtrl& c1, const ClientCtrl& c2) {
    return  c1.client_type == c2.client_type &&
            c1.num_game_thread_used == c2.num_game_thread_used &&
            c1.player_swap == c2.player_swap && 
            c1.async == c2.async;
  }

  friend bool operator!=(const ClientCtrl& c1, const ClientCtrl& c2) {
    return !(c1 == c2);
  }
};

/*
  Used model version.
*/ 
struct MsgVersion {
  int64_t model_ver;

  MsgVersion(int ver = -1) : model_ver(ver) {}
};


enum RestartReply {
  NO_OP,
  ONLY_WAIT,
  UPDATE_REQUEST_ONLY,
  UPDATE_MODEL,
  UPDATE_MODEL_ASYNC,
  UPDATE_COMPLETE,
};


struct MsgRestart {
  RestartReply  result;
  int           game_idx;

  MsgRestart(RestartReply res = NO_OP, int game_idx = -1)
      : result(res), game_idx(game_idx) { }
};


struct MsgRequest {
  ModelPair   vers;
  ClientCtrl  client_ctrl;

  void setJsonFields(json& j) const {
    JSON_SAVE_OBJ(j, vers);
    JSON_SAVE_OBJ(j, client_ctrl);
  }

  static MsgRequest createFromJson(const json& j) {
    MsgRequest  request;
    JSON_LOAD_OBJ(request, j, vers);
    JSON_LOAD_OBJ_ARGS(request, j, client_ctrl, request.vers.is_selfplay());
    return request;
  }

  std::string setJsonFields() const {
    json      j;
    setJsonFields(j);
    return j.dump();
  }

  std::string info() const {
    std::stringstream ss;
    ss  << client_ctrl.info();
    ss  << vers.info();
    return ss.str();
  }

  friend bool operator==(const MsgRequest& m1, const MsgRequest& m2) {
    return m1.vers == m2.vers && m1.client_ctrl == m2.client_ctrl;
  }

  friend bool operator!=(const MsgRequest& m1, const MsgRequest& m2) {
    return !(m1 == m2);
  }
};


struct MsgRequestSeq {
  int64_t seq = -1;
  MsgRequest request;

  void setJsonFields(json& j) const {
    JSON_SAVE_OBJ(j, request);
    JSON_SAVE(j, seq);
  }

  static MsgRequestSeq createFromJson(const json& j) {
    MsgRequestSeq s;
    JSON_LOAD_OBJ(s, j, request);
    JSON_LOAD(s, j, seq);
    return s;
  }
  std::string dumpJsonString() const {
    json j;
    setJsonFields(j);
    return j.dump();
  }

  std::string info() const {
    std::stringstream ss;
    
    ss << "[seq=" << seq << "]" << request.info();
    return ss.str();
  }
};


/* 
  Information about 1 game thread.
*/  
struct ThreadState {
  int thread_id = -1;
  // Which game we have played.
  int seq = 0;
  // Which move we have proceeded.
  int move_idx = 0;

  // Player versions
  int64_t black = -1;
  int64_t white = -1;

  void setJsonFields(json& j) const {
    JSON_SAVE(j, thread_id);
    JSON_SAVE(j, seq);
    JSON_SAVE(j, move_idx);
    JSON_SAVE(j, black);
    JSON_SAVE(j, white);
  }

  static ThreadState createFromJson(const json& j) {
    ThreadState state;
    JSON_LOAD(state, j, thread_id);
    JSON_LOAD(state, j, seq);
    JSON_LOAD(state, j, move_idx);
    JSON_LOAD(state, j, black);
    JSON_LOAD(state, j, white);
    return state;
  }

  friend bool operator==(const ThreadState& t1, const ThreadState& t2) {
    return t1.thread_id == t2.thread_id && t1.seq == t2.seq &&
        t1.move_idx == t2.move_idx && t1.black == t2.black &&
        t1.white == t2.white;
  }

  friend bool operator!=(const ThreadState& t1, const ThreadState& t2) {
    return !(t1 == t2);
  }

  std::string info() const {
    std::stringstream ss;
    ss  << "ThreadState::info()\t\t[th_id=" << thread_id << "]"
        << "[seq=" << seq << "]"
        << "[mv_idx=" << move_idx << "]"
        << "[black=" << black << "]"
        << "[white=" << white << "]\n";
    return ss.str();
  }
};

