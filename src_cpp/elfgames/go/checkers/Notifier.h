#pragma once

#include "../mcts/mcts.h"
#include "elf/ai/tree_search/mcts.h"
#include "CheckersStateExt.h"
#include "../common/record.h"


struct CheckersGuardedRecords {
 public:
  CheckersGuardedRecords(const std::string& identity)
      : records_(identity),
        logger_(elf::logging::getLogger(
            "elfgames::checkers::Notifier::CheckersGuardedRecords",
            "")) {
    display_debug_info("struct CheckersGuardedRecords", __FUNCTION__, RED_B);        
  }

  void feed(const CheckersStateExt& s) {
    display_debug_info("struct CheckersGuardedRecords", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);
    records_.addRecord(s.dumpRecord());
  }

  void updateState(const ThreadState& ts) {
    display_debug_info("struct CheckersGuardedRecords", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);

    auto now = elf_utils::sec_since_epoch_from_now();
    records_.updateState(ts);

    last_states_.push_back(std::make_pair(now, ts));
    if (last_states_.size() > 100) {
      last_states_.pop_front();
    }

    if (now - last_state_vis_time_ > 60) {
      std::unordered_map<int, ThreadState> states;
      std::unordered_map<int, uint64_t> timestamps;
      for (const auto& s : last_states_) {
        timestamps[s.second.thread_id] = s.first;
        states[s.second.thread_id] = s.second;
      }

      logger_->info(
          "CheckersGuardedRecords::updateState[{}] {}",
          elf_utils::now(),
          visStates(states, &timestamps));

      last_state_vis_time_ = now;
    }
  }

  size_t size() {
    display_debug_info("struct CheckersGuardedRecords", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);
    return records_.records.size();
  }

  std::string dumpAndClear() {
    display_debug_info("struct CheckersGuardedRecords", __FUNCTION__, RED_B);

    // send data.
    std::lock_guard<std::mutex> lock(mutex_);
    logger_->info(
        "CheckersGuardedRecords::DumpAndClear[{}], #records: {}, {}",
        elf_utils::now(),
        records_.records.size(),
        visStates(records_.states));

    std::string s = records_.dumpJsonString();
    records_.clear();
    return s;
  }

 private:
  std::mutex mutex_;
  Records records_;
  std::deque<std::pair<uint64_t, ThreadState>> last_states_;
  uint64_t last_state_vis_time_ = 0;
  std::shared_ptr<spdlog::logger> logger_;


  static std::string visStates(const std::unordered_map<int, ThreadState>& states, const std::unordered_map<int, uint64_t>* timestamps = nullptr) {
    display_debug_info("struct CheckersGuardedRecords", __FUNCTION__, RED_B);

    std::stringstream ss;
    ss << "#states: " << states.size();
    ss << "[";

    auto now = elf_utils::sec_since_epoch_from_now();
    std::vector<int> ordered;
    for (const auto& p : states) {
      ordered.push_back(p.first);
    }
    std::sort(ordered.begin(), ordered.end());

    for (const auto& th_id : ordered) {
      auto it = states.find(th_id);
      assert(it != states.end());

      ss << th_id << ":" << it->second.seq << ":" << it->second.move_idx;

      if (timestamps != nullptr) {
        auto it = timestamps->find(th_id);
        if (it != timestamps->end()) {
          uint64_t td = now - it->second;
          ss << ":" << td;
        }
        ss << ",";
      }
    }
    ss << "]  ";

    ss << elf_utils::get_gap_list(ordered);
    return ss.str();
  }
};
















class CheckersGameNotifier {
 public:
  CheckersGameNotifier(
      Ctrl& ctrl,
      const std::string& identity,
      const GameOptions& options,
      elf::GameClient* client)
      : ctrl_(ctrl), records_(identity), options_(options), client_(client) {
    display_debug_info("CheckersGameNotifier", __FUNCTION__, RED_B);

    using std::placeholders::_1;
    using std::placeholders::_2;

    ctrl.RegCallback<std::pair<int, std::string>>(
        std::bind(&CheckersGameNotifier::dump_records, this, _1, _2));
  }

  void OnGameEnd(const CheckersStateExt& s) override {
    display_debug_info("CheckersGameNotifier", __FUNCTION__, RED_B);

    // tell python / remote
    records_.feed(s);

    game_stats_.resetRankingIfNeeded(options_.num_reset_ranking);
    game_stats_.feedWinRate(s.state().getFinalValue());
    // game_stats_.feedSgf(s.dumpSgf(""));

    // Report winrate (so that Python side could know).
    elf::FuncsWithState funcs =
        client_->BindStateToFunctions({end_target_}, &s);
    client_->sendWait({end_target_}, &funcs);
  }

  void OnStateUpdate(const ThreadState& state) override {
    display_debug_info("CheckersGameNotifier", __FUNCTION__, RED_B);

    // Update current state.
    records_.updateState(state);
  }

  void OnMCTSResult(Coord c, const CheckersGameNotifierBase::MCTSResult& result)
      override {
    display_debug_info("CheckersGameNotifier", __FUNCTION__, RED_B);

    // Check the ranking of selected move.
    auto move_rank =
        result.getRank(c, elf::ai::tree_search::MCTSResultT<Coord>::PRIOR);
    game_stats_.feedMoveRanking(move_rank.first);
  }

  GameStats& getGameStats() {
    display_debug_info("CheckersGameNotifier", __FUNCTION__, RED_B);

    return game_stats_;
  }

 private:
  Ctrl& ctrl_;
  GameStats game_stats_;
  CheckersGuardedRecords records_;
  const GameOptions options_;
  elf::GameClient* client_ = nullptr;
  const std::string end_target_ = "game_end";

  bool dump_records(const Addr&, std::pair<int, std::string>& data) {
    display_debug_info("CheckersGameNotifier", __FUNCTION__, RED_B);

    data.first = records_.size();
    data.second = records_.dumpAndClear();
    return true;
  }
};