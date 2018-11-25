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

#include "elfgames/go/checkers/CheckersState.h"

using AI = elf::ai::AIClientT<CheckersFeature, CheckersReply>;

namespace elf {
namespace ai {
namespace tree_search {

template <>
struct ActionTrait<int> {
 public:
  static std::string to_string(const int& c) {
    return "[" + coord2str2(c) + "][" + coord2str(c) + "][" +
        std::to_string(c) + "]";
  }
  static int default_value() {
    display_debug_info("ActionTrait", __FUNCTION__, RED_B);

    return M_INVALID;
  }
};

template <>
struct StateTrait<CheckersState, int> {
 public:
  static std::string to_string(const Checkers& s) {
    return "tt score (no komi): " + std::to_string(s.evaluate(0));
  }
  static bool equals(const Checkers& s1, const Checkers& s2) {
    return s1.getHashCode() == s2.getHashCode();
  }

  static bool moves_since(
      const Checkers& s,
      size_t* next_move_number,
      std::vector<int>* moves) {
    return s.moves_since(next_move_number, moves);
  }
};

} // namespace tree_search
} // namespace ai
} // namespace elf
