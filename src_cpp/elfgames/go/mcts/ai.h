/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "elf/ai/ai.h"
#include "elf/ai/tree_search/tree_search_base.h"

#include "elfgames/go/base/go_state.h"

using AI = elf::ai::AIClientT<BoardFeature, GoReply>;

namespace elf {
namespace ai {
namespace tree_search {

template <>
struct ActionTrait<Coord> {
 public:
  static std::string to_string(const Coord& c) {
    display_debug_info("ai.h ActionTrait", __FUNCTION__, RED_B);

    return "[" + coord2str2(c) + "][" + coord2str(c) + "][" +
        std::to_string(c) + "]";
  }
  static Coord default_value() {
    display_debug_info("ai.h ActionTrait", __FUNCTION__, RED_B);

    return M_INVALID;
  }
};

template <>
struct StateTrait<GoState, Coord> {
 public:
  static std::string to_string(const GoState& s) {
    display_debug_info("ai.h StateTrait", __FUNCTION__, RED_B);

    return "tt score (no komi): " + std::to_string(s.evaluate(0));
  }
  static bool equals(const GoState& s1, const GoState& s2) {
    display_debug_info("ai.h StateTrait", __FUNCTION__, RED_B);

    // std::cout << "s1 : " << &s1 << std::endl;
    // std::cout << "s2 : " << &s2 << std::endl;
    // std::cout << std::endl;
    // std::cout << "s1.getHashCode() : " << s1.getHashCode() << std::endl;
    // std::cout << "s2.getHashCode() : " << s2.getHashCode() << std::endl;
    // std::cout << std::endl;

    // std::cout << "s1 : " << std::endl << s1.showBoard() << s1.getPly() << std::endl;
    // std::cout << "s2 : " << std::endl << s2.showBoard() << s2.getPly() << std::endl;

    // std::cout << "_ply\t: " << s1.board()._ply << " == " << s2.board()._ply << std::endl;
    // std::cout << "_num_groups\t: " << s1.board()._num_groups << " == " << s2.board()._num_groups << std::endl;
    // std::cout << "_b_cap\t: " << s1.board()._b_cap << " == " << s2.board()._b_cap << std::endl;
    // std::cout << "_rollout_passes\t: " << s1.board()._rollout_passes << " == " << s2.board()._rollout_passes << std::endl;
    // std::cout << "_last_move\t: " << s1.board()._last_move << " == " << s2.board()._last_move << std::endl;
    // std::cout << "_last_move2\t: " << s1.board()._last_move2 << " == " << s2.board()._last_move2 << std::endl;
    // std::cout << "_last_move3\t: " << s1.board()._last_move3 << " == " << s2.board()._last_move3 << std::endl;
    // std::cout << "_last_move4\t: " << s1.board()._last_move4 << " == " << s2.board()._last_move4 << std::endl;
    // std::cout << "_num_group_removed\t: " << s1.board()._num_group_removed << " == " << s2.board()._num_group_removed << std::endl;
    // std::cout << "_ko_age\t: " << s1.board()._ko_age << " == " << s2.board()._ko_age << std::endl;
    // std::cout << "_simple_ko\t: " << s1.board()._simple_ko << " == " << s2.board()._simple_ko << std::endl;
    // std::cout << "_simple_ko_color\t: " << s1.board()._simple_ko_color << " == " << s2.board()._simple_ko_color << std::endl;
    // std::cout << "_next_player\t: " << s1.board()._next_player << " == " << s2.board()._next_player << std::endl;
    

    // std::cout << "hash_equals == " << (s1.getHashCode() == s2.getHashCode()) << std::endl;
    // std::cout << std::endl;
    return s1.getHashCode() == s2.getHashCode();
  }

  static bool moves_since(
      const GoState& s,
      size_t* next_move_number,
      std::vector<Coord>* moves) {
    display_debug_info("ai.h StateTrait", __FUNCTION__, RED_B);

    for (auto i = moves->begin(); i != moves->end(); ++i)
      std::cout << *i << ", ";

    return s.moves_since(next_move_number, moves);
  }
};

} // namespace tree_search
} // namespace ai
} // namespace elf


