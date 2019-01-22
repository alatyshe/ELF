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

#include "../checkers/CheckersState.h"

using CheckersAI = elf::ai::AIClientT<CheckersFeature, CheckersReply>;

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
		return "Score : " + std::to_string(s.evaluateGame());
	}
	static bool equals(const CheckersState& s1, const CheckersState& s2) {
		CheckersBoard b1 = s1.board();
		CheckersBoard b2 = s2.board();

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
		res += (b1._last_move_green != b2._last_move_green);
		res += (b1._last_move_red != b2._last_move_red);

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









