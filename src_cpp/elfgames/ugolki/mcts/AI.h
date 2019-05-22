/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iomanip>
#include <utility>

// elf
#include "elf/ai/ai.h"
#include "elf/ai/tree_search/tree_search_base.h"

#include "../game/GameState.h"

using AIClientT = elf::ai::AIClientT<BoardFeature, BoardReply>;

namespace elf {
namespace ai {
namespace tree_search {

template <>
struct ActionTrait<Coord> {
 public:
  static std::string to_string(const Coord& c) {
    return "[" + std::to_string(c) + "]";
  }

  static Coord default_value() {
    return M_INVALID;
  }
};


template <>
struct StateTrait<GameState, Coord> {
 public:
  static std::string to_string(const GameState& s) {
    return "Score Current Board: " + std::to_string(s.evaluateGame());
  }

  static bool equals(const GameState& s1, const GameState& s2) {
    return CompareBoards(s1.board(), s2.board());
  }

  static bool moves_since(
      const GameState& s,
      size_t* next_move_number,
      std::vector<Coord>* moves) {
    return s.moves_since(next_move_number, moves);
  }
};

} // namespace tree_search
} // namespace ai
} // namespace elf

