/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "ServerGameTrain.h"

ServerGameTrain::ServerGameTrain(
    int game_idx,
    elf::GameClient* client,
    const ContextOptions& context_options,
    const CheckersGameOptions& options,
    elf::shared::ReaderQueuesT<CheckersRecord>* reader)
      : GameBase(game_idx, client, context_options, options), 
        reader_(reader),
        logger_(elf::logging::getIndexedLogger(
          std::string("\x1b[1;35;40m|++|\x1b[0m") + 
          "ServerGameTrain-" + std::to_string(game_idx) + "-",
          "")) {
  for (size_t i = 0; i < kNumState; ++i) {
    _checkers_state_ext.emplace_back(new CheckersStateExtOffline(game_idx, options));
  }
  logger_->info("Was succefully created");
}

void ServerGameTrain::act() {  
  std::vector<elf::FuncsWithState> funcsToSend;

  for (size_t i = 0; i < kNumState; ++i) {
    while (true) {
      int q_idx;
      auto sampler = reader_->getSamplerWithParity(&_rng, &q_idx);
      const CheckersRecord* r = sampler.sample();
      if (r == nullptr) {
        continue;
      }
      _checkers_state_ext[i]->fromRecord(*r);

      // Random pick one ply.
      if (_checkers_state_ext[i]->switchRandomMove(&_rng))
        break;
    }

    funcsToSend.push_back(
        client_->BindStateToFunctions({"train"}, _checkers_state_ext[i].get()));
  }

  // client_->sendWait({"train"}, &funcs);

  std::vector<elf::FuncsWithState*> funcPtrsToSend(funcsToSend.size());
  for (size_t i = 0; i < funcsToSend.size(); ++i) {
    funcPtrsToSend[i] = &funcsToSend[i];
  }

  // VERY DANGEROUS - sending pointers of local objects to a function
  client_->sendBatchWait({"train"}, funcPtrsToSend);
}
