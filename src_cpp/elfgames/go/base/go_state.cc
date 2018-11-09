/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <chrono>
#include <fstream>

#include "board_feature.h"
#include "go_state.h"

static std::vector<std::string> split(const std::string& s, char delim) {
  display_debug_info("", __FUNCTION__, BLUE_B);
  
  std::cout << "const std::string& s : " << s << std::endl;
  std::cout << "char delim : |" << delim << "|" << std::endl;

  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}

static Coord s2c(const std::string& s) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  std::cout << "const std::string& s : " << s << std::endl;

  int row = s[0] - 'A';
  if (row >= 9)
    row--;
  int col = stoi(s.substr(1)) - 1;
  return getCoord(row, col);
}

///////////// GoState ////////////////////
bool GoState::forward(const Coord& c) {
  display_debug_info("GoState", __FUNCTION__, RED_B);

  std::cout << "const Coord& c : " << c << std::endl;

  if (c == M_INVALID) {
    throw std::range_error("GoState::forward(): move is M_INVALID");
  }
  if (terminated())
    return false;

  GroupId4 ids;
  if (!TryPlay2(&_board, c, &ids))
    return false;

  // _add_board_hash(c);

  Play(&_board, &ids);

  _moves.push_back(c);
  _history.emplace_back(_board);
  if (_history.size() > MAX_NUM_AGZ_HISTORY)
    _history.pop_front();
  return true;
}

bool GoState::_check_superko() const {
  display_debug_info("GoState", __FUNCTION__, RED_B);

  // Check superko rule.
  // need to check whether last move is pass or not.
  if (lastMove() == M_PASS)
    return false;

  // uint64_t key = _board._hash;
  // auto it = _board_hash.find(key);
  // if (it != _board_hash.end()) {
  //   for (const auto& r : it->second) {
  //     if (isBitsEqual(_board._bits, r.bits))
  //       return true;
  //   }
  // }
  return false;
}

// void GoState::_add_board_hash(const Coord& c) {
//   display_debug_info("GoState", __FUNCTION__, RED_B);

//   std::cout << "const Coord& c : " << c << std::endl;

//   if (c == M_PASS)
//     return;

//   uint64_t key = _board._hash;
//   auto& r = _board_hash[key];
//   r.emplace_back();
//   copyBits(r.back().bits, _board._bits);
// }

bool GoState::checkMove(const Coord& c) const {
  display_debug_info("GoState", __FUNCTION__, RED_B);

  std::cout << "const Coord& c : " << c << std::endl;

  GroupId4 ids;
  if (c == M_INVALID)
    return false;
  return TryPlay2(&_board, c, &ids);
}


void GoState::reset() {
  display_debug_info("GoState", __FUNCTION__, RED_B);
  
  clearBoard(&_board);
  _moves.clear();
  // _board_hash.clear();
  _history.clear();
  _final_value = 0.0;
  _has_final_value = false;
}

