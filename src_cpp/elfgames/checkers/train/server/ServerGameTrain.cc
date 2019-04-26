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
    const GameOptions& game_options,
    elf::shared::ReaderQueuesT<GameRecord>* readerQueues)
      : GameBase(game_idx, client, context_options, game_options), 
        readerQueues_(readerQueues),
        logger_(elf::logging::getIndexedLogger(
          MAGENTA_B + std::string("|++|") + COLOR_END + 
          "ServerGameTrain-" + std::to_string(game_idx) + "-",
          "")) {
  for (size_t i = 0; i < kNumState; ++i) {
    _game_state_ext.emplace_back(new GameStateExtOffline(game_idx, game_options));
  }
  logger_->info("Was succefully created");
}

void ServerGameTrain::act() {  
  std::vector<elf::FuncsWithState> funcsToSend;

  for (size_t i = 0; i < kNumState; ++i) {
    while (true) {
      int q_idx;
      auto sampler = readerQueues_->getSamplerWithParity(&_rng, &q_idx);

      const GameRecord* r = sampler.sample();
      if (r == nullptr) {
        continue;
      }
      _game_state_ext[i]->fromRecord(*r);
      // Random pick one ply.
      if (_game_state_ext[i]->switchRandomMove(&_rng))
        break;
    }

    funcsToSend.push_back(
        client_->BindStateToFunctions({"train"}, _game_state_ext[i].get()));
  }

  // client_->sendWait({"train"}, &funcs);
  std::vector<elf::FuncsWithState*> funcPtrsToSend(funcsToSend.size());
  for (size_t i = 0; i < funcsToSend.size(); ++i) {
    funcPtrsToSend[i] = &funcsToSend[i];
  }

  // VERY DANGEROUS - sending pointers of local objects to a function
  client_->sendBatchWait({"train"}, funcPtrsToSend);
}
