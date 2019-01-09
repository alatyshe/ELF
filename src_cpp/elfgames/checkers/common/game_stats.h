/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "elf/logging/IndexedLoggerFactory.h"
#include "game_utils.h"

// отображается иногда на клиенте
// Total count: 5582
// [0]: 2464 (44.1419%)
// [1]: 1624 (29.0935%)
// [2]: 319 (5.7148%)
// [3]: 252 (4.51451%)
// [4]: 350 (6.27015%)
// [5]: 324 (5.80437%)
// [6]: 189 (3.38588%)
// [7]: 42 (0.752418%)
// [8]: 8 (0.143318%)
// [9]: 5 (0.0895736%)
// [10]: 4 (0.0716589%)
// ??????????????????????????????????
// ??????????????????????????????????
// ??????????????????????????????????
// ??????????????????????????????????
// ==========================================================
// ==========================================================
class GameStats {
 public:
	GameStats()
			: _logger(
						elf::logging::getIndexedLogger(
							std::string("\x1b[1;35;40m|++|\x1b[0m") + 
							"GameStats-", 
							"")) {}

	// ????????????
	void feedMoveRanking(int ranking) {
		display_debug_info("GameStats", __FUNCTION__, RED_B, false);

		std::lock_guard<std::mutex> lock(_mutex);
		_move_ranking.feed(ranking);
	}
	// ????????????
	void resetRankingIfNeeded(int num_reset_ranking) {
		display_debug_info("GameStats", __FUNCTION__, RED_B, false);

		std::lock_guard<std::mutex> lock(_mutex);
		if (_move_ranking.total_count > (uint64_t)num_reset_ranking) {
			_logger->info("\n{}", _move_ranking.info());
			_move_ranking.reset();
		}
	}


	void feedWinRate(float final_value) {
		display_debug_info("GameStats", __FUNCTION__, RED_B, false);

		std::lock_guard<std::mutex> lock(_mutex);
		_win_rate_stats.feed(final_value);
	}

	// void feedSgf(const std::string& sgf) {
	//   display_debug_info("GameStats", __FUNCTION__, RED_B, false);

	//   std::lock_guard<std::mutex> lock(_mutex);
	//   _sgfs.push_back(sgf);
	// }

	// For sender.
	WinRateStats getWinRateStats() {
		display_debug_info("GameStats", __FUNCTION__, RED_B, false);

		std::lock_guard<std::mutex> lock(_mutex);
		return _win_rate_stats;
	}

	// std::vector<std::string> getPlayedGames() {
	//   display_debug_info("GameStats", __FUNCTION__, RED_B, false);
		
	//   std::lock_guard<std::mutex> lock(_mutex);
	//   return _sgfs;
	// }

 private:
	std::mutex		_mutex;
	Ranking				_move_ranking;
	WinRateStats	_win_rate_stats;
	// std::vector<std::string> _sgfs;
	std::shared_ptr<spdlog::logger> _logger;
};
