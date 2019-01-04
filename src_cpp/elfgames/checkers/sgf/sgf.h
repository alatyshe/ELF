/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "elf/logging/IndexedLoggerFactory.h"

#include "../checkers/CheckersBoard.h"

// constexpr int BOARD_SIZE = 9;
// constexpr int BOARD_EXPAND_SIZE = BOARD_SIZE + 2;
// constexpr int BOUND_COORD = BOARD_EXPAND_SIZE * BOARD_EXPAND_SIZE;

// Load the remaining part.
inline Coord str2coord(const std::string& s) {
  return (std::stoi(s));
}

inline std::string coord2str(Coord c) {
  return std::to_string(c);
}

// переводим координаты в строку
inline std::string coords2str(const std::vector<Coord>& moves) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  std::string sgf = "(";
  for (size_t i = 0; i < moves.size(); i++) {
    // std::string color = i % 2 == 0 ? "B" : "W";
    // + color 
    sgf += ";[" + coord2str(moves[i]) + "]";
  }
  sgf += ")";
  return sgf;
}

// (;B[fg];W[de];B[ei];W[bh];B[gc];W[ea];B[di];W[ag];B[gg];W[gh];B[cb];W[ca];B[ac];W[gf];B[hh];W[fd];B[id];W[bi];B[eb];W[af];B[ia];W[eg];B[fa];W[cg];B[ae];W[bb];B[ed];W[ch];B[gb];W[hi];B[cc];W[aa];B[hg];W[fe];B[ce];W[dg];B[if];W[ih];B[gd];W[gi];B[ie];W[ge];B[ec];W[ai];B[hd];W[cf];B[ic];W[df];B[hc];W[fi];B[db];W[ff];B[he];W[fb];B[cd];W[ig];B[dh];W[bg];B[be];W[eh];B[ee];W[ba];B[dd];W[bf];B[fh];W[ab];B[bc];W[ad];B[ii];W[gi];B[hi];W[ef];B[ih];W[ci];B[fi];W[ha];B[gh];W[ah];B[gi];W[fc];B[da];W[hb];B[];W[ib];B[];W[bb];B[];W[ia];B[];W[ab];B[ba];W[];B[])
// (;[32];[88];[4];[84];[11];[137];[98];[143];[104];[90];[9];[159];[102];[67];[7];[65];[2];[69];[30];[97];[10];[142];[154];[36];[144];[139];[100];[68];[14];[71];[42];[149];[35];[161];[9];[65];[28];[137];[116];[92];[8];[159];[11];[161])

inline std::vector<Coord> str2coords(const std::string& sgf) {
  display_debug_info("", __FUNCTION__, BLUE_B);
  
  std::vector<Coord> moves;
  if (sgf.empty() || sgf[0] != '(')
    return moves;

  size_t i = 1;
  while (true) {
    if (sgf[i] != ';')
      break;
    while (i < sgf.size() && sgf[i] != '[')
      i++;
    if (i == sgf.size())
      break;

    i++;
    size_t j = i;

    while (j < sgf.size() && sgf[j] != ']')
      j++;
    if (j == sgf.size())
      break;

    moves.push_back(str2coord(sgf.substr(i, j - i)));
    i = j + 1;
  }

  return moves;
}
