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
// checkers
#include "../checkers/CheckersState.h"

using AIClientT = elf::ai::AIClientT<CheckersFeature, CheckersReply>;

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
struct StateTrait<CheckersState, Coord> {
 public:
  static std::string to_string(const CheckersState& s) {
    return "Score Current Board: " + std::to_string(s.evaluateGame());
  }
  static bool equals(const CheckersState& s1, const CheckersState& s2) {
    CheckersBoard b1 = s1.board();
    CheckersBoard b2 = s2.board();

    int res = 0;
    
    for (int y = 0; y < CHECKERS_BOARD_SIZE; y++) {
      for (int x = 0; x < CHECKERS_BOARD_SIZE; x++)
        res += (b1.board[y][x] != b1.board[y][x]);
    }
    res += (b1.current_player != b2.current_player);
    res += (b1.game_ended != b2.game_ended);
    res += (b1.jump_y != b2.jump_y);
    res += (b1.jump_x != b2.jump_x);
    res += (b1.white_must_leave_base != b2.white_must_leave_base);
    res += (b1.black_must_leave_base != b2.black_must_leave_base);
    res += (b1._last_move != b2._last_move);
    res += (b1._ply != b2._ply);

    return res == 0;

  }

  static bool moves_since(
      const CheckersState& s,
      size_t* next_move_number,
      std::vector<Coord>* moves) {
    return s.moves_since(next_move_number, moves);
  }
};

} // namespace tree_search
} // namespace ai
} // namespace elf

