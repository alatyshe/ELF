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
#include "elfgames/go/base/board.h"
#include "elfgames/go/base/common.h"

// Load the remaining part.
inline Coord str2coord(const std::string& s) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  if (s.size() < 2)
    return M_PASS;
  size_t i = 0;
  while (i < s.size() && (s[i] == '\n' || s[i] == ' '))
    i++;
  if (i == s.size())
    return M_INVALID;
  int x = s[i] - 'a';

  i++;
  while (i < s.size() && (s[i] == '\n' || s[i] == ' '))
    i++;
  if (i == s.size())
    return M_INVALID;
  // if (x >= 9) x --;
  int y = s[i] - 'a';

  // tt
  if (!ON_BOARD(x, y))
    return M_INVALID;
  // if (y >= 9) y --;
  return OFFSETXY(x, y);
}

inline std::string coord2str(Coord c) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  if (c == M_PASS)
    return "";
  int x = X(c);
  // if (x >= 8) x ++;
  int y = Y(c);
  // if (y >= 8) y ++;

  return std::string{static_cast<char>('a' + x), static_cast<char>('a' + y)};
}

inline std::string player2str(Stone player) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  switch (player) {
    case S_WHITE:
      return "W";
    case S_BLACK:
      return "B";
    case S_UNKNOWN:
      return "U";
    case S_OFF_BOARD:
      return "O";
    default:
      return "-";
  }
}

inline std::string coord2str2(Coord c) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  if (c == M_PASS)
    return "PASS";
  int x = X(c);
  if (x >= 8)
    x++;
  int y = Y(c);

  std::string s{static_cast<char>('A' + x)};
  return s + std::to_string(y + 1);
}

inline std::string coords2sgfstr(const std::vector<Coord>& moves) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  std::string sgf = "(";
  for (size_t i = 0; i < moves.size(); i++) {
    std::string color = i % 2 == 0 ? "B" : "W";
    sgf += ";" + color + "[" + coord2str(moves[i]) + "]";
  }
  sgf += ")";
  return sgf;
}

inline std::vector<Coord> sgfstr2coords(const std::string& sgf) {
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
