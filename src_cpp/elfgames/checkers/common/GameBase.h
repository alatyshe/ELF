/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

// ELF
#include "elf/base/context.h"
#include "elf/legacy/python_options_utils_cpp.h"
#include "elf/logging/IndexedLoggerFactory.h"
#include "elf/utils/utils.h"

// Checkers
#include "GameFeature.h"

// От этого класса наследуется ClientGameSelfplay и ServerGameTrain
// и их создается необходимое
// количество (параметр num_games) в классе GameContext.
// запускается mainLoop
// ==========================================================
// ==========================================================
class GameBase {
 public:
	GameBase(
			int game_idx,
			elf::GameClient* client,
			const ContextOptions& context_options,
			const CheckersGameOptions& options)
			: client_(client),
				_game_idx(game_idx),
				_options(options),
				_context_options(context_options),
				_logger(elf::logging::getIndexedLogger(
							std::string("\x1b[1;35;40m|++|\x1b[0m") + 
							"GameBase-", 
							"")) {
		if (options.seed == 0) {
			_seed = elf_utils::get_seed(
					game_idx ^ std::hash<std::string>{}(context_options.job_id));
		} else {
			_seed = options.seed;
		}
		_rng.seed(_seed);
	}

	// 
	void mainLoop() {
		if (_options.verbose) {
			_logger->info(
					"mainLoop was started [{}] Seed: {}, thread_id: {}",
					_game_idx,
					_seed,
					std::hash<std::thread::id>{}(std::this_thread::get_id()));
		}
		// Main loop of the game.
		// Делаем бесконечный цикл, пока клиент/сервер не сообщит что надо остановиться
		// параметр --suicide_after_n_games 1
		while (!client_->DoStopGames()) {
			act();
		}
	}

	virtual void act() 	= 0;
	virtual ~GameBase() = default;

 protected:
	elf::GameClient*      client_ = nullptr;
	uint64_t              _seed = 0;
	std::mt19937          _rng;

	int                   _game_idx = -1;

	CheckersGameOptions   _options;
	ContextOptions        _context_options;

 private:
	std::shared_ptr<spdlog::logger> _logger;
};





