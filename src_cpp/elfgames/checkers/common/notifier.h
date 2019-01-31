#pragma once

#include "../mcts/CheckersMCTSActor.h"
#include "elf/ai/tree_search/mcts.h"
#include "record.h"
#include "../checkers/Record.h"

// ==========================================================
// ==========================================================
class CheckersGameNotifierBase {
 public:
  using MCTSResult = elf::ai::tree_search::MCTSResultT<Coord>;
  virtual void OnGameEnd(const CheckersStateExt&) {}
  virtual void OnStateUpdate(const ThreadState&) {}
  virtual void OnMCTSResult(Coord, const MCTSResult&) {}
};