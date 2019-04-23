/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// elf
#include "elf/base/context.h"
#include "elf/legacy/python_options_utils_cpp.h"
#include "elf/logging/IndexedLoggerFactory.h"
#include "elf/utils/utils.h"

#include "GameFeature.h"

/*
  Base class. from this class inherited ClientGameSelfplay and ServerGameTrain
  Creates the N number of copies in class GameContext,
  specified in the parameter --num_games.
  ClientGameSelfplay - generates batches.
  ServerGameTrain - train model.

  Important, contains elf::GameClient.
*/
class GameBase {
 public:
  GameBase(
      int game_idx,
      elf::GameClient* client,
      const ContextOptions& context_options,
      const GameOptions& game_options)
      : client_(client),
        _game_idx(game_idx),
        _game_options(game_options),
        _context_options(context_options),
        _logger(elf::logging::getIndexedLogger(
              MAGENTA_B + std::string("|++|") + COLOR_END + 
              "GameBase-", 
              "")) {
    if (game_options.seed == 0) {
      _seed = elf_utils::get_seed(
          game_idx ^ std::hash<std::string>{}(context_options.job_id));
    } else {
      _seed = game_options.seed;
    }
    _rng.seed(_seed);
  }

  void mainLoop() {
    if (_game_options.verbose) {
      _logger->info(
          "mainLoop was started [{}] Seed: {}, thread_id: {}",
          _game_idx,
          _seed,
          std::hash<std::thread::id>{}(std::this_thread::get_id()));
    }
    /*
      Main loop of the game.
      Run loop till client/server won't tell to stop.
      The parameter is responsible for this - --suicide_after_n_games(client side).
    */
    while (!client_->DoStopGames()) {
      act();
    }
  }

  virtual void act() = 0;
  virtual ~GameBase() = default;

 protected:
  elf::GameClient* client_ = nullptr;
  uint64_t _seed = 0;
  std::mt19937 _rng;

  int _game_idx = -1;

  GameOptions _game_options;
  ContextOptions _context_options;

 private:
  std::shared_ptr<spdlog::logger> _logger;
};


