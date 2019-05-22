/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <atomic>

// elf
#include "elf/logging/IndexedLoggerFactory.h"

#include "../common/record.h"

class ClientManager;

/* 
	Онформация по одному клиенту с которым сервер имеет связь
*/
struct ClientInfo {
 public:
	enum ClientChange { ALIVE2DEAD, DEAD2ALIVE, ALIVE, DEAD };

	struct State {
		mutable std::mutex		mutex_;
		ThreadState						last_state_;
		uint64_t							last_state_update_ = 0;
		const ClientManager&	mgr_;

		State(const ClientManager& mgr);
		bool CompareGame(const ModelPair& p) const;
		bool StateUpdate(const ThreadState& ts);
	};

	ClientInfo(
			const ClientManager&	mgr,
			const std::string&		id,
			int 									num_threads,
			int										max_delay_sec);

	const ClientManager& getManager() const {
		return mgr_;
	}

	const std::string& id() const {
		return identity_;
	}

	int seq() const {
		return seq_.load();
	}

	bool justAllocated() const {
		return seq_ == 0;
	}

	void incSeq() {
		seq_++;
	}

	ClientType type() const {
		std::lock_guard<std::mutex> lock(mutex_);

		return type_;
	}

	void set_type(ClientType t) {
		std::lock_guard<std::mutex> lock(mutex_);
		type_ = t;
	}

	bool IsActive() const {
		std::lock_guard<std::mutex> lock(mutex_);

		return active_;
	}

	bool IsStuck(uint64_t curr_timestamp, uint64_t* delay = nullptr) const {
		std::lock_guard<std::mutex> lock(mutex_);
		auto last_delay = curr_timestamp - last_update_;

		if (delay != nullptr)
			*delay = last_delay;
		return last_delay >= max_delay_sec_;
	}

	void stateUpdate(const ThreadState& ts);

	ClientChange updateActive();

	const State& threads(int thread_id) const {
		std::lock_guard<std::mutex> lock(mutex_);
		
		assert(thread_id >= 0 && thread_id < (int)threads_.size());
		return *threads_[thread_id];
	}

 private:
	mutable std::mutex		mutex_;
	const ClientManager&	mgr_;
	const std::string			identity_;
	ClientType						type_ = CLIENT_INVALID;

	uint64_t							max_delay_sec_ = 300;

	std::atomic<int64_t>	seq_;

	bool									active_ = true;
	uint64_t							last_update_ = 0;
	std::vector<std::unique_ptr<State>> threads_;
};




/*
	контроллим тут всех наших клиентов.
	создаем новых и говорим в каком режиме работать(EVAL_THEN_SELFPLAY/SELFPLAY_ONLY).
*/
class ClientManager {
 public:
	ClientManager(
			int				max_num_threads,
			uint64_t	max_client_delay_sec,
			int				num_expected_clients,
			float			selfplay_only_ratio = 0.9,
			int				max_num_eval = -1,
			std::function<uint64_t()> timer = elf_utils::sec_since_epoch_from_now)
			:	max_num_threads_(max_num_threads),
				max_client_delay_sec_(max_client_delay_sec),
				num_expected_clients_(num_expected_clients),
				selfplay_only_ratio_(selfplay_only_ratio),
				max_num_eval_(max_num_eval),
				timer_(timer),
				logger_(elf::logging::getIndexedLogger(
						MAGENTA_B + std::string("|++|") + COLOR_END + 
						"ClientManager-",
						"")) {
		assert(timer_ != nullptr);
	}

	void setSelfplayOnlyRatio(float ratio) {
		std::lock_guard<std::mutex> lock(mutex_);
		selfplay_only_ratio_ = ratio;
	}

	const ClientInfo& updateStates(
			const std::string& identity,
			const std::unordered_map<int, ThreadState>& states) {
		std::lock_guard<std::mutex> lock(mutex_);

		// Достаем нашего клиента по названию, для обновления информации о нем
		ClientInfo& info = _getClient(identity);

		for (const auto& s : states) {
			info.stateUpdate(s.second);
		}

		// A client is considered dead after 20 min.
		updateClients();
		return info;
	}

	const ClientInfo* getClientC(const std::string& identity) const {
		std::lock_guard<std::mutex> lock(mutex_);
		auto it = clients_.find(identity);

		if (it != clients_.end()) {
			return it->second.get();
		} else {
			return nullptr;
		}
	}

	ClientInfo& getClient(const std::string& identity) {
		std::lock_guard<std::mutex> lock(mutex_);

		return _getClient(identity);
	}

	size_t getNumEval() const {
		std::lock_guard<std::mutex> lock(mutex_);

		return num_eval_then_selfplay_;
	}

	size_t getExpectedNumEval() const {
		std::lock_guard<std::mutex> lock(mutex_);

		// num_expected_clients_ 	- ожидаемое количество клиентов(если они установлены)
		// selfplay_only_ratio_ 	- %клиентов работающих в режиме генерации батчей
		if (num_expected_clients_ > 0) {
			// Возвращаем ожидаемое количество клиентов,
			// которые работают в режиме eval_than_selfplay
			return num_expected_clients_ * (1.0 - selfplay_only_ratio_);
		} else {
			// в противном случае возращаем текущее количество 
			// клиентов работающих в режиме eval_than_selfplay
			return num_eval_then_selfplay_;
		}
	}

	uint64_t getCurrTimeStamp() const {
		return timer_();
	}

	std::string info() const {
		std::lock_guard<std::mutex> lock(mutex_);
		return _info();
	}

 private:
 	// num clients that run only in selplay mode, without comparing models
	float						selfplay_only_ratio_;
	const int				num_expected_clients_;
	const int				max_num_eval_;
	const int				max_num_threads_;
	const uint64_t	max_client_delay_sec_;

	std::function<uint64_t()> timer_ = nullptr;

	mutable std::mutex mutex_;
	std::unordered_map<std::string, std::unique_ptr<ClientInfo>> clients_;
	int							num_selfplay_only_ = 0;
	int							num_eval_then_selfplay_ = 0;

	std::shared_ptr<spdlog::logger> logger_;



	std::string _info() const {
		std::stringstream ss;
		int n = num_selfplay_only_ + num_eval_then_selfplay_;

		ss << "Total\t\t\tclients[" << n << "==100%]" << std::endl
			 << "SelfplayOnly\t\tclients[" << num_selfplay_only_ << "/"
			 << 100 * static_cast<float>(num_selfplay_only_) / n << "%]"<< std::endl
			 << "EvalThenSelfplay\tclients[" << num_eval_then_selfplay_ << "/"
			 << 100 * static_cast<float>(num_eval_then_selfplay_) / n << "%]"<< std::endl
			 << "[#expected_clients=" << num_expected_clients_ << "]"
			 << "[selfplay_only_ratio=" << selfplay_only_ratio_ << "%]"
			 << "[#max_eval_clients=" << max_num_eval_ << "]"
			 << "[#max_threads=" << max_num_threads_ << "]"
			 << "[#client_delay=" << max_client_delay_sec_ << "]";

		return ss.str();
	}

	float curr_selfplay_ratio() const {
		return static_cast<float>(num_selfplay_only_) /
				(num_selfplay_only_ + num_eval_then_selfplay_ + 1e-10);
	}

	ClientType alloc_type() {
		ClientType t = CLIENT_INVALID;

		// МОЕ
		// if (num_eval_then_selfplay_ + num_selfplay_only_ == 0)
		// {
		// 	t = CLIENT_EVAL_THEN_SELFPLAY;
		// 	num_eval_then_selfplay_++;
		// 	return t;
		// }
		// 
		if (curr_selfplay_ratio() >= selfplay_only_ratio_ &&
				(max_num_eval_ < 0 || num_eval_then_selfplay_ < max_num_eval_)) {
			t = CLIENT_EVAL_THEN_SELFPLAY;
			num_eval_then_selfplay_++;
		} else {
			t = CLIENT_SELFPLAY_ONLY;
			num_selfplay_only_++;
		}
		return t;
	}

	void dealloc_type(ClientType t) {
		if (t == CLIENT_EVAL_THEN_SELFPLAY)
			num_eval_then_selfplay_--;
		else if (t == CLIENT_SELFPLAY_ONLY)
			num_selfplay_only_--;
	}

	void updateClients() {
		std::vector<std::string> newly_dead;
		std::vector<std::string> newly_alive;

		for (auto& p : clients_) {
			auto& c = *p.second;
			auto status = c.updateActive();
			if (status == ClientInfo::ALIVE2DEAD) {
				newly_dead.push_back(p.first);
				dealloc_type(c.type());
			} else if (status == ClientInfo::DEAD2ALIVE) {
				newly_alive.push_back(p.first);
				c.set_type(alloc_type());
			}
		}

		if (!newly_dead.empty() || !newly_alive.empty()) {
			logger_->info(
					"{} Client newly dead: {}, newly alive: {}, {}",
					getCurrTimeStamp(),
					newly_dead.size(),
					newly_alive.size(),
					_info());
			for (const auto& s : newly_dead) {
				logger_->warn("Newly dead: {}", s);
			}
			for (const auto& s : newly_alive) {
				logger_->info("Newly alive: {}", s);
			}
		}
	}

	ClientInfo& _getClient(const std::string& identity) {
		auto it = clients_.find(identity);

		if (it != clients_.end())
			return *it->second;

		auto& e = clients_[identity];
		e.reset(new ClientInfo(
				*this, identity, max_num_threads_, max_client_delay_sec_));
		e->set_type(alloc_type());
		return *e;
	}
};
