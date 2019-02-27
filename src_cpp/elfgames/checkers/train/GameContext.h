/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>
#include <memory>
#include <vector>

// elf
#include "elf/base/context.h"
#include "elf/legacy/python_options_utils_cpp.h"
#include "elf/logging/IndexedLoggerFactory.h"
// checkers
#include "../common/ClientGameSelfPlay.h"
#include "../common/DistriClient.h"
#include "../common/record.h"
#include "../mcts/AI.h"
#include "data_loader.h"
#include "server/DistriServer.h"
#include "server/ServerGameTrain.h"

#include <thread>

/*
	The context of the game wrapped by python GameContext.
	Contains:
	Context - configures data exchange between С++ and Python, allocates shared memmory.
	GameBase - vector of games. The number of which is set by parameter --num_games.
	DistriServer - .
	DistriClient - .

	GameFeature - Registers the keys by which Python will access memory in C++ 
			as well as the methods that will be called while accessing these keys.

	logger_ - displays log info in terminal.
*/
class GameContext {
 public:
	using ThreadedDispatcher = ClientGameSelfPlay::ThreadedDispatcher;

	GameContext(const ContextOptions& contextOptions, 
							const CheckersGameOptions& gameOptions)
			: gameFeature_(gameOptions),
				logger_(elf::logging::getIndexedLogger(
					MAGENTA_B + std::string("|++|") + COLOR_END + 
					"GameContext-", 
					"")) {
		context_.reset(new elf::Context);

		int numGames = contextOptions.num_games;
		const int batchsize = contextOptions.batchsize;

		// Register all keys "checkers_s", "checkers_V" etc.
		gameFeature_.registerExtractor(batchsize, context_->getExtractor());

		elf::GameClient* gameClient = context_->getClient();
		ThreadedDispatcher* dispatcher = nullptr;

		// Creates the necessary number of games. Each game has its own unique id.
		if (gameOptions.mode == "train" || gameOptions.mode == "offline_train") {
			// start_server.sh
			server_.reset(new DistriServer(contextOptions, gameOptions, gameClient));

			for (int i = 0; i < numGames; ++i) {
				games_.emplace_back(new ServerGameTrain(
						i,
						gameClient,
						contextOptions,
						gameOptions,
						server_->getReplayBuffer()));
			}
			logger_->info("{} ServerGameTrain was created", numGames);
		} else {
			// start_client.sh
			client_.reset(new DistriClient(contextOptions, gameOptions, gameClient));
			dispatcher = client_->getDispatcher();

			for (int i = 0; i < numGames; ++i) {
				games_.emplace_back(new ClientGameSelfPlay(
						i,
						gameClient,
						contextOptions,
						gameOptions,
						dispatcher,
						client_->getCheckersNotifier()));
			}
			logger_->info("{} ClientGameSelfPlay was created", numGames);
		}


		context_->setStartCallback(
				numGames, 
				[this, dispatcher] (int i, elf::GameClient*) {
					if (dispatcher != nullptr) {
						dispatcher->RegGame(i);
					}
					games_[i]->mainLoop();
				});

		if (server_ != nullptr) {
			/* 
				Registers the lambda function that is called when the server starts.
				It uses batches from the file(if specified) for training.
				GameContext => DistriServer => DataOnlineLoader =>
			*/
			context_->setCBAfterGameStart(
					[this, gameOptions] () { server_->loadOfflineSelfplayData(); }
					);
		}
	}

	std::map<std::string, int> getParams() const {
		return gameFeature_.getParams();
	}

	const GameBase* getGame(int game_idx) const {
		if (_check_game_idx(game_idx)) {
			logger_->error("Invalid game_idx [{}]", game_idx);
			return nullptr;
		}
		return games_[game_idx].get();
	}

	elf::Context* ctx() {
		return context_.get();
	}

	DistriServer*	getServer() {
		return server_.get();
	}

	DistriClient*	getClient() {
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
		return game_idx < 0 || game_idx >= (int)games_.size();
	}

 private:
	std::unique_ptr<elf::Context> context_;
	std::vector<std::unique_ptr<GameBase>> games_;

	std::unique_ptr<DistriServer> server_;
	std::unique_ptr<DistriClient> client_;

	GameFeature gameFeature_;

	std::shared_ptr<spdlog::logger> logger_;
};
