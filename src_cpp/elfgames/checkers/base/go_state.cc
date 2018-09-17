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
  std::stringstream ss(s);
  std::string item;
  std::vector<std::string> elems;
  while (getline(ss, item, delim)) {
    elems.push_back(std::move(item));
  }
  return elems;
}
