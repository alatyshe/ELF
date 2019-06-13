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

// elf
#include "elf/legacy/pybind_helper.h"
#include "elf/logging/IndexedLoggerFactory.h"

struct WinRateStats {
  uint64_t black_wins = 0;
  uint64_t white_wins = 0;
  uint64_t both_reach_base = 0;
  uint64_t both_lost = 0;
  float sum_reward = 0.0;
  uint64_t total_games = 0;

  void feed(FinishReason reason, float reward) {
    if (reason == MAX_STEP)
      both_lost++;
    else if (reason == BOTH_REACHED_BASE)
      both_reach_base++;
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
    both_reach_base = 0;
    sum_reward = 0.0;
    total_games = 0;
  }

  REGISTER_PYBIND_FIELDS(black_wins, white_wins, both_reach_base, both_lost, sum_reward, total_games);
};


class GameStats {
 public:
  GameStats()
      : _logger(elf::logging::getIndexedLogger(
            MAGENTA_B + std::string("|++|") + COLOR_END + 
            "GameStats-", 
            "")) {}

  void feedWinRate(FinishReason reason, float final_value) {
    std::lock_guard<std::mutex> lock(_mutex);
    _win_rate_stats.feed(reason, final_value);
  }

  // For sender.
  WinRateStats getWinRateStats() {
    std::lock_guard<std::mutex> lock(_mutex);
    return _win_rate_stats;
  }

 private:
  std::mutex _mutex;
  WinRateStats _win_rate_stats;
  std::shared_ptr<spdlog::logger> _logger;
};
