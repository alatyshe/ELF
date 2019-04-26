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

#include "record.h"
#include "../game/Record.h"
#include "../mcts/MCTSGameActor.h"


/*
  from this class inherit GameNotifier.
*/
class GameNotifierBase {
 public:
  using MCTSResult = elf::ai::tree_search::MCTSResultT<Coord>;
  virtual void OnGameEnd(const GameStateExt&) {}
  virtual void OnStateUpdate(const ThreadState&) {}
};