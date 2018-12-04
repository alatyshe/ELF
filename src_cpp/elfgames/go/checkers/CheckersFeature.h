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

#include "CheckersBoard.h"
#include "elf/logging/IndexedLoggerFactory.h"

// просто история досок и все пока не исаользуется
struct CheckersBoardHistory {

  std::array<int64_t, 2>  forward;
  std::array<int64_t, 2>  backward;
  std::array<int64_t, 2>  pieces;
  int64_t                 empty;
  int                     active;

  // просто копируем положение фишек на доске
  CheckersBoardHistory(const CheckersBoard& b) {
    display_debug_info("struct CheckersBoardHistory", __FUNCTION__, "\x1b[1;36;40m");

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
    display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[1;36;40m");
  }

  const CheckersState& state() const {
    display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[1;36;40m");
    return s_;
  }

  // int64_t coord2Action(int m) const {
  //   display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[1;36;40m");

  //   if (m == M_PASS)
  //     return BOARD_ACTION_PASS;
  //   return EXPORT_OFFSET_XY(X(m), Y(m));
  // }

  // int action2Coord(int64_t action) const {
  //   display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[1;36;40m");
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
  // bool getStones(int player, float* data) const;
  // bool getHistory(int player, float* data) const;
  // bool getHistoryExp(int player, float* data) const;
};











struct CheckersReply {
  const CheckersFeature& bf;
  int c;
  std::vector<float> pi;
  float value = 0;
  // Model version.
  int64_t version = -1;

  CheckersReply(const CheckersFeature& bf) : bf(bf), pi(TOTAL_NUM_ACTIONS, 0.0) {
    display_debug_info("struct CheckersReply", __FUNCTION__, "\x1b[1;36;40m");
  }
  
  void    info() {
    std::cout << "c : " << c << std::endl;
    std::cout << "pi : " << std::endl;
    for (auto i = pi.begin(); i != pi.end(); ++i)
      std::cout << *i << ", ";
    std::cout << std::endl;
    std::cout << "value : " << value << std::endl;
    std::cout << "version : " << version << std::endl;
    // std::cout << bf.state().showBoard() << std::endl;
  }
};





