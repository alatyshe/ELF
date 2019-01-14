#pragma once

#include <time.h>

#include <iostream>
#include <memory>
#include <vector>

// ELF
#include "elf/base/context.h"
#include "elf/legacy/python_options_utils_cpp.h"
#include "elf/logging/IndexedLoggerFactory.h"

// Checkers
#include "../distri_base.h"
#include "../../common/record.h"
#include "../data_loader.h"
#include "train_ctrl.h"

#include <thread>

class DistriServer {
 public:
	DistriServer(
			const ContextOptions& 			contextOptions,
			const CheckersGameOptions& 	gameOptions,
			elf::GameClient* 						client)
			: contextOptions_(contextOptions),
				gameOptions_(gameOptions),
				logger_(elf::logging::getIndexedLogger(
					std::string("\x1b[1;35;40m|++|\x1b[0m") + 
					"DistriServer-", 
					"")) {
		auto netOptions = getNetOptions(contextOptions_, gameOptions_);

		trainCtrl_.reset(new TrainCtrl(
				ctrl_,
				contextOptions_.num_games,
				client,
				gameOptions_,
				contextOptions_.mcts_options));
		logger_->info(
				"Finished initializing trainCtrl_(TrainCtrl)");

		if (gameOptions_.mode == "train") {
			// создаем экземпляр для подгрузки батчей онлайн

			onlineLoader_.reset(new DataOnlineLoader(netOptions));
			logger_->info(
				"Finished initializing onlineLoader_(DataOnlineLoader)");

			onlineLoader_->start(trainCtrl_.get());
		} else if (gameOptions_.mode == "offline_train") {
			// подразумевается что данные у нас уже есть
			;
		} else {
			throw std::range_error("options.mode not recognized! " + gameOptions_.mode);
		}
		logger_->info(
			"{}DistriServer successfully created{}\n",
			GREEN_B, 
			COLOR_END);
	}



	ReplayBuffer* getReplayBuffer() {
		return trainCtrl_->getReplayBuffer();
	}

	void ServerWaitForSufficientSelfplay(int64_t selfplay_ver) {
		trainCtrl_->getThreadedCtrl()->waitForSufficientSelfplay(selfplay_ver);
	}

	// Used in training side.
	void notifyNewVersion(int64_t selfplay_ver, int64_t new_version) {
		trainCtrl_->getThreadedCtrl()->addNewModelForEvaluation(
				selfplay_ver, new_version);
	}

	void setInitialVersion(int64_t init_version) {
		trainCtrl_->getThreadedCtrl()->setInitialVersion(init_version);
	}

	void setEvalMode(int64_t new_ver, int64_t old_ver) {
		trainCtrl_->setEvalMode(new_ver, old_ver);
	}

	~DistriServer() {
		trainCtrl_.reset(nullptr);
		onlineLoader_.reset(nullptr);
	}

	void loadOfflineSelfplayData() {
		if (gameOptions_.list_files.empty())
			return;

		std::atomic<int> count(0);
		const size_t numThreads = 16;

		auto thread_main = [this, &count](size_t idx) {
			for (size_t k = 0; k * numThreads + idx < gameOptions_.list_files.size();
					 ++k) {
				const std::string& f = gameOptions_.list_files[k * numThreads + idx];
				logger_->info("Loading offline data, reading file {}", f);

				std::string content;
				if (!CheckersRecord::loadContent(f, &content)) {
					logger_->error("Offline data loader: error reading {}", f);
					return;
				}
				trainCtrl_->OnReceive("", content);
			}
		};

		std::vector<std::thread> threads;
		for (size_t i = 0; i < numThreads; ++i) {
			threads.emplace_back(std::bind(thread_main, i));
		}

		for (auto& t : threads) {
			t.join();
		}

		logger_->info(
				"All offline data is loaded. Read {} records from {} files. Reader "
				"info {}",
				count,
				gameOptions_.list_files.size(),
				trainCtrl_->getReplayBuffer()->info());
	}

 private:
	Ctrl ctrl_;

	std::unique_ptr<TrainCtrl>				trainCtrl_;
	std::unique_ptr<DataOnlineLoader>	onlineLoader_;

	const ContextOptions							contextOptions_;
	const CheckersGameOptions					gameOptions_;

	std::shared_ptr<spdlog::logger> 	logger_;
};
