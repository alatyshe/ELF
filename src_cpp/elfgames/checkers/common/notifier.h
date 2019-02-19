/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// elf
#include "elf/ai/tree_search/mcts.h"

// Checkers
#include "record.h"
#include "../checkers/Record.h"
#include "../mcts/CheckersMCTSActor.h"

class CheckersGameNotifierBase {
 public:
  using MCTSResult = elf::ai::tree_search::MCTSResultT<Coord>;
  virtual void OnGameEnd(const CheckersStateExt&) {}
  virtual void OnStateUpdate(const ThreadState&) {}
  virtual void OnMCTSResult(Coord, const MCTSResult&) {}
};