/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "../../common/GameBase.h"
#include "elf/distributed/shared_reader.h"

/* 
  server side
*/
class ServerGameTrain : public GameBase {
 public:
  ServerGameTrain(
      int game_idx,
      elf::GameClient* client,
      const ContextOptions& context_options,
      const GameOptions& game_options,
      elf::shared::ReaderQueuesT<GameRecord>* readerQueues);

  void act() override;

 private:
  elf::shared::ReaderQueuesT<GameRecord>* readerQueues_ = nullptr;

  static constexpr size_t kNumState = 64;
  std::vector<std::unique_ptr<GameStateExtOffline>> _game_state_ext;

  std::shared_ptr<spdlog::logger> logger_;
};
