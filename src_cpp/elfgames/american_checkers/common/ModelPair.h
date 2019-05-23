/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <nlohmann/json.hpp>

// elf
#include "elf/ai/tree_search/tree_search_options.h"
#include "elf/utils/json_utils.h"
// game
#include "../game/GameBoard.h"

/* 
  Model pair used for evaluation 2 models on the client with EVAL_THAN_SELFPLAY type.
*/
struct ModelPair {
  int64_t black_ver = -1;
  int64_t white_ver = -1;
  elf::ai::tree_search::TSOptions mcts_opt;

  bool wait() const {
    return black_ver < 0;
  }

  void set_wait() {
    black_ver = white_ver = -1;
  }

  bool is_selfplay() const {
    return black_ver >= 0 && white_ver == -1;
  }

  std::string info() const {
    std::stringstream ss;

    ss << "\nModelPair::info()\t";
    if (wait())
      ss  << "[wait]";
    else if (is_selfplay())
      ss  << "[selfplay_version=" << black_ver << "]";
    else {
      ss  << "[" << GREEN_C << "Black_v" << COLOR_END << "=" << black_ver << "]["
          << RED_C << "White_v" << COLOR_END << "=" << white_ver << "]";
    }
    ss << mcts_opt.info();
    return ss.str();
  }

  friend bool operator==(const ModelPair& p1, const ModelPair& p2) {
    return p1.black_ver == p2.black_ver && p1.white_ver == p2.white_ver &&
        p1.mcts_opt == p2.mcts_opt;
  }

  friend bool operator!=(const ModelPair& p1, const ModelPair& p2) {
    return !(p1 == p2);
  }

  void setJsonFields(json& j) const {
    JSON_SAVE(j, black_ver);
    JSON_SAVE(j, white_ver);
    JSON_SAVE_OBJ(j, mcts_opt);
  }

  static ModelPair createFromJson(const json& j) {
    ModelPair p;

    JSON_LOAD(p, j, black_ver);
    JSON_LOAD(p, j, white_ver);
    JSON_LOAD_OBJ(p, j, mcts_opt);
    return p;
  }
};


// ==========================================================
// ==========================================================
namespace std {
template <>
struct hash<ModelPair> {
  typedef ModelPair argument_type;
  typedef std::size_t result_type;

  result_type operator()(argument_type const& s) const noexcept {
    result_type const h1(std::hash<int64_t>{}(s.black_ver));
    result_type const h2(std::hash<int64_t>{}(s.white_ver));
    result_type const h3(
        std::hash<elf::ai::tree_search::TSOptions>{}(s.mcts_opt));
    return h1 ^ (h2 << 1) ^ (h3 << 2);
  }
};
} // namespace std
