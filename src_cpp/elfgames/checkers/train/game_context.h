/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// TODO: Figure out how to remove this (ssengupta@fb)
#include <time.h>

#include <iostream>
#include <memory>
#include <vector>

// ELF
#include "elf/base/context.h"
#include "elf/legacy/python_options_utils_cpp.h"
#include "elf/logging/IndexedLoggerFactory.h"

// Checkers
#include "../common/GameSelfPlay.h"
#include "../common/record.h"
#include "../mcts/AI.h"

#include "data_loader.h"
#include "game_train.h"
#include "client/distri_client.h"
#include "server/distri_server.h"

#include <thread>

class GameContext {
 public:
  using ThreadedDispatcher = GameSelfPlay::ThreadedDispatcher;

  GameContext(const ContextOptions&       contextOptions, 
              const CheckersGameOptions&  options)
      : goFeature_(options),
        logger_(elf::logging::getIndexedLogger("GameContext-", "")) {
    display_debug_info("GameContext", __FUNCTION__, RED_B);

    context_.reset(new elf::Context);

    int numGames = contextOptions.num_games;
    const int batchsize = contextOptions.batchsize;

    // Register all functions.
    goFeature_.registerExtractor(batchsize, context_->getExtractor());

    elf::GameClient* gc = context_->getClient();
    ThreadedDispatcher* dispatcher = nullptr;

    if (options.mode == "train" || options.mode == "offline_train") {
      server_.reset(new Server(contextOptions, options, gc));

      for (int i = 0; i < numGames; ++i) {
        games_.emplace_back(new GameTrain(
            i, gc, contextOptions, options, server_->getReplayBuffer()));
      }
    } else {

      client_.reset(new Client(contextOptions, options, gc));

      dispatcher = client_->getDispatcher();
      
      for (int i = 0; i < numGames; ++i) {
        // создаем необходимое количество игр
        // у каждой игры свой уникальный id - i
        // для доступа к ней
        games_.emplace_back(new GameSelfPlay(
            i,
            gc,
            contextOptions,
            options,
            dispatcher,
            client_->getCheckersNotifier()
            ));
      }
    }

    context_->setStartCallback(
        numGames, [this, dispatcher](int i, elf::GameClient*) {
          if (dispatcher != nullptr) {
            dispatcher->RegGame(i);
          }
          games_[i]->mainLoop();
        });

    if (server_ != nullptr) {
      context_->setCBAfterGameStart(
          [this, options]() { server_->loadOfflineSelfplayData(); });
    }
  }

  std::map<std::string, int> getParams() const {
    display_debug_info("GameContext", __FUNCTION__, RED_B);

    return goFeature_.getParams();
  }

  const GameBase* getGame(int game_idx) const {
    display_debug_info("GameContext", __FUNCTION__, RED_B);

    if (_check_game_idx(game_idx)) {
      logger_->error("Invalid game_idx [{}]", game_idx);
      return nullptr;
    }

    return games_[game_idx].get();
  }

  elf::Context* ctx() {
    display_debug_info("GameContext", __FUNCTION__, RED_B);

    return context_.get();
  }

  Server* getServer() {
    display_debug_info("GameContext", __FUNCTION__, RED_B);

    return server_.get();
  }

  Client* getClient() {
    display_debug_info("GameContext", __FUNCTION__, RED_B);

    return client_.get();
  }

  ~GameContext() {
    server_.reset(nullptr);
    client_.reset(nullptr);
    games_.clear();
    context_.reset(nullptr);
  }

 private:
  bool _check_game_idx(int game_idx) const {
    display_debug_info("GameContext", __FUNCTION__, RED_B);

    return game_idx < 0 || game_idx >= (int)games_.size();
  }

 private:
  std::unique_ptr<elf::Context>           context_;
  std::vector<std::unique_ptr<GameBase>>  games_;

  std::unique_ptr<Server>                 server_;
  std::unique_ptr<Client>                 client_;

  GoFeature                               goFeature_;

  std::shared_ptr<spdlog::logger>         logger_;
};
