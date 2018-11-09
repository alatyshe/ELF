/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <random>
#include <vector>

// #include "../base/go_common.h"
#include "CheckersBoard.h"
// #include "CheckersState.h"


#include "elf/logging/IndexedLoggerFactory.h"

// #define MAX_NUM_FEATURE 25

// #define OUR_LIB 0
// #define OPPONENT_LIB 3
// #define OUR_SIMPLE_KO 6
// #define OUR_STONES 7
// #define OPPONENT_STONES 8
// #define EMPTY_STONES 9

// // [TODO]: Other todo features.
// #define OUR_HISTORY 10
// #define OPPONENT_HISTORY 11
// #define BORDER 12
// #define POSITION_MARK 13
// #define OUR_CLOSEST_COLOR 14
// #define OPPONENT_CLOSEST_COLOR 15

// #define BLACK_INDICATOR 16
// #define WHITE_INDICATOR 17

// #define MAX_NUM_AGZ_FEATURE 18
// #define MAX_NUM_AGZ_HISTORY 8

// просто история досок и все
struct CheckersBoardHistory {

  std::array<int64_t, 2>  forward;
  std::array<int64_t, 2>  backward;
  std::array<int64_t, 2>  pieces;
  int64_t                 empty;
  int                     active;

  // просто копируем положение фишек на доске
  CheckersBoardHistory(const CheckersBoard& b) {
    display_debug_info("struct CheckersBoardHistory", __FUNCTION__, "\x1b[2;30;43m");

    forward[BLACK_PLAYER] = b.forward[BLACK_PLAYER];
    backward[BLACK_PLAYER] = b.backward[BLACK_PLAYER];
    pieces[BLACK_PLAYER] = (forward[BLACK_PLAYER]) | (backward[BLACK_PLAYER]);
    forward[WHITE_PLAYER] = b.forward[WHITE_PLAYER];
    backward[WHITE_PLAYER] = b.backward[WHITE_PLAYER];
    pieces[WHITE_PLAYER] = (forward[WHITE_PLAYER]) | (backward[WHITE_PLAYER]);
    empty = empty;
    active = active;
  }
};

class CheckersState;


















class CheckersFeature {
 public:

  CheckersFeature(const CheckersState& s) 
      : s_(s)
      // ,
      // logger_(
      //     elf::logging::getLogger("elfgames::go::base::CheckersFeature-", ""))
    {
    display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[2;30;43m");
  }

  // const CheckersState& state() const {
  //   display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[2;30;43m");
  //   return s_;
  // }

  // std::pair<int, int> InvTransform(const std::pair<int, int>& p) const {
  //   display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[2;30;43m");

  //   std::pair<int, int> output(p);

  //   output = std::make_pair(output.second, BOARD_SIZE - output.first - 1);
  //   return output;
  // }

  // int64_t coord2Action(int m) const {
  //   display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[2;30;43m");

  //   if (m == M_PASS)
  //     return BOARD_ACTION_PASS;
  //   return EXPORT_OFFSET_XY(X(m), Y(m));
  // }

  // int action2Coord(int64_t action) const {
  //   display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[2;30;43m");
  //   if (action == -1 || action == BOARD_ACTION_PASS)
  //     return M_PASS;
  //   auto p = InvTransform(std::make_pair(EXPORT_X(action), EXPORT_Y(action)));
  //   return OFFSETXY(p.first, p.second);
  // }

  void extract(std::vector<float>* features) const;
  void extract(float* features) const;

 private:
  const CheckersState& s_;

  static constexpr int64_t kBoardRegion = CHECKERS_BOARD_SIZE * CHECKERS_BOARD_SIZE;


  // Compute features.
  void getPawns(int player, float* data) const;
  void getKings(int player, float* data) const;
  // bool getLibertyMap3binary(int player, float* data) const;
  // bool getStones(int player, float* data) const;
  // bool getSimpleKo(int player, float* data) const;
  // bool getHistory(int player, float* data) const;
  // bool getHistoryExp(int player, float* data) const;
  // bool getDistanceMap(int player, float* data) const;
};









struct CheckersReply {
  const CheckersFeature& bf;
  int c;
  std::vector<float> pi;
  float value = 0;
  // Model version.
  int64_t version = -1;

  CheckersReply(const CheckersFeature& bf) : bf(bf), pi(TOTAL_NUM_ACTIONS, 0.0) {
    display_debug_info("struct CheckersReply", __FUNCTION__, "\x1b[2;30;43m");
  }
};

