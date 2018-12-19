/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include "go_common.h"

#include <random>
#include <vector>

#include "elf/logging/IndexedLoggerFactory.h"

#define MAX_NUM_FEATURE 25

#define OUR_LIB 0
#define OPPONENT_LIB 3
#define OUR_SIMPLE_KO 6
#define OUR_STONES 7
#define OPPONENT_STONES 8
#define EMPTY_STONES 9

// [TODO]: Other todo features.
#define OUR_HISTORY 10
#define OPPONENT_HISTORY 11
#define BORDER 12
#define POSITION_MARK 13
#define OUR_CLOSEST_COLOR 14
#define OPPONENT_CLOSEST_COLOR 15

#define BLACK_INDICATOR 16
#define WHITE_INDICATOR 17

#define MAX_NUM_AGZ_FEATURE 18
#define MAX_NUM_AGZ_HISTORY 8

struct BoardHistory {
  std::vector<Coord> black;
  std::vector<Coord> white;

  BoardHistory(const Board& b) {
    display_debug_info("struct BoardHistory", __FUNCTION__, RED_B);
    for (int i = 0; i < BOARD_SIZE; ++i) {
      for (int j = 0; j < BOARD_SIZE; ++j) {
        Coord c = OFFSETXY(i, j);
        Stone s = b._infos[c].color;
        if (s == S_WHITE)
          white.push_back(c);
        else if (s == S_BLACK)
          black.push_back(c);
      }
    }
  }
};

class GoState;


















class BoardFeature {
 public:

  BoardFeature(const GoState& s) 
      : s_(s),
      logger_(
          elf::logging::getIndexedLogger("elfgames::go::base::BoardFeature-", ""))
      {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);
  }

  const GoState& state() const {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);
    return s_;
  }

  std::pair<int, int> InvTransform(const std::pair<int, int>& p) const {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);

    std::pair<int, int> output(p);

    output = std::make_pair(output.second, BOARD_SIZE - output.first - 1);
    return output;
  }

  int64_t coord2Action(Coord m) const {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);

    if (m == M_PASS)
      return BOARD_ACTION_PASS;
    return EXPORT_OFFSET_XY(X(m), Y(m));
  }

  Coord action2Coord(int64_t action) const {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);
    if (action == -1 || action == BOARD_ACTION_PASS)
      return M_PASS;
    auto p = InvTransform(std::make_pair(EXPORT_X(action), EXPORT_Y(action)));
    return OFFSETXY(p.first, p.second);
  }

  void extract(std::vector<float>* features) const;
  void extract(float* features) const;

 private:
  const GoState& s_;

  static constexpr int64_t kBoardRegion = BOARD_SIZE * BOARD_SIZE;

  std::shared_ptr<spdlog::logger> logger_;

  int transform(int x, int y) const {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);

    return EXPORT_OFFSET_XY(x, y);
  }

  int transform(Coord m) const {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);

    return transform(X(m), Y(m));
  }

  int transform(Coord m, int c) const {
    display_debug_info("BoardFeature", __FUNCTION__, RED_B);
    
    return transform(X(m), Y(m)) + c * kBoardRegion;
  }

  // Compute features.
  bool getLibertyMap3binary(Stone player, float* data) const;
  bool getStones(Stone player, float* data) const;
  bool getSimpleKo(Stone player, float* data) const;
  bool getHistory(Stone player, float* data) const;
  bool getHistoryExp(Stone player, float* data) const;
  bool getDistanceMap(Stone player, float* data) const;
};







