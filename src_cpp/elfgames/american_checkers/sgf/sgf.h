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

// elf
#include "elf/logging/IndexedLoggerFactory.h"
// game
#include "../game/GameBoard.h"

/*
  Translates data into a string and vice versa
*/
// Load the remaining part.
inline Coord str2coord(const std::string& s) {
  return (std::stoi(s));
}

inline std::string coord2str(Coord c) {
  return std::to_string(c);
}

inline std::string coords2str(const std::vector<Coord>& moves) {
  std::string sgf = "(";
  for (size_t i = 0; i < moves.size(); i++) {
    sgf += ";[" + coord2str(moves[i]) + "]";
  }
  sgf += ")";
  return sgf;
}

inline std::vector<Coord> str2coords(const std::string& sgf) {  
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
