/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <map>
#include <random>
#include "elf/legacy/pybind_helper.h"

// ????????????????????????????????????
// ????????????????????????????????????
// ????????????????????????????????????
// Просто сбор статистки, но какой
// ==========================================================
// ==========================================================
// struct Ranking {

// 	std::vector<uint64_t>   counts;
// 	uint64_t                total_count;

// 	Ranking(int max_rank = 10) : counts(max_rank + 1), total_count(0) {}

// 	void feed(int r) {
// 		if (r < (int)counts.size())
// 			counts[r]++;
// 		total_count++;
// 	}

// 	void reset() {
// 		std::fill(counts.begin(), counts.end(), 0);
// 		total_count = 0;
// 	}

// 	std::string info() const {		
// 		std::stringstream ss;

// 		ss << "Total count: " << total_count << std::endl;
// 		for (size_t i = 0; i < counts.size(); ++i) {
// 			ss << "[" << i << "]: " << counts[i] << " ("
// 				 << 100.0 * counts[i] / total_count << "%)" << std::endl;
// 		}

// 		return ss.str();
// 	}
// };

// ==========================================================
// ==========================================================
struct WinRateStats {
	uint64_t	black_wins = 0;
	uint64_t	white_wins = 0;
	uint64_t	both_lost = 0;
	float			sum_reward = 0.0;
	uint64_t	total_games = 0;

	void feed(CheckersFinishReason reason, float reward) {
		if (reason == CHECKERS_MAX_STEP)
			both_lost++;
		else if (reward > 0)
			black_wins++;
		else
			white_wins++;
		sum_reward += reward;
		total_games++;
	}

	void reset() {
		black_wins = 0;
		white_wins = 0;
		both_lost = 0;
		sum_reward = 0.0;
		total_games = 0;
	}

	REGISTER_PYBIND_FIELDS(black_wins, white_wins, both_lost, sum_reward, total_games);
};
