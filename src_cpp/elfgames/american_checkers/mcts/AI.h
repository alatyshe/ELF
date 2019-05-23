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
// game
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
    GameBoard b1 = s1.board();
    GameBoard b2 = s2.board();

    int res = 0;
    res += (b1.forward[0] != b2.forward[0]);
    res += (b1.forward[1] != b2.forward[1]);
    res += (b1.backward[0] != b2.backward[0]);
    res += (b1.backward[1] != b2.backward[1]);
    res += (b1.pieces[0] != b2.pieces[0]);
    res += (b1.pieces[1] != b2.pieces[1]);
    res += (b1.empty != b2.empty);
    res += (b1.active != b2.active);
    res += (b1.passive != b2.passive);
    res += (b1.jump != b2.jump);
    res += (b1._last_move != b2._last_move);
    res += (b1._ply != b2._ply);

    // repeat moves
    res += (b1._last_move_black[0] != b2._last_move_black[0]);
    res += (b1._last_move_black[1] != b2._last_move_black[1]);
    res += (b1._last_move_white[0] != b2._last_move_white[0]);
    res += (b1._last_move_white[1] != b2._last_move_white[1]);
    res += (b1._remove_step_black != b2._remove_step_black);
    res += (b1._remove_step_white != b2._remove_step_white);
    res += (b1._black_repeats_step != b2._black_repeats_step);
    res += (b1._white_repeats_step != b2._white_repeats_step);

    return res == 0;
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

