/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <assert.h>
#include <algorithm>
#include <deque>

#include "../common/go_game_specific.h"
#include "client_manager.h"
#include "ctrl_utils.h"

#include "elf/ai/tree_search/tree_search_options.h"
#include "elf/logging/IndexedLoggerFactory.h"

using TSOptions = elf::ai::tree_search::TSOptions;


struct SelfPlayRecord {
 public:
  SelfPlayRecord(int ver, const GameOptions& options)
      : ver_(ver),
        options_(options),
        logger_(elf::logging::getLogger(
            "elfgames::go::train::SelfPlayRecord-",
            "")) {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    std::string selfplay_prefix =
        "selfplay-" + options_.server_id + "-" + options_.time_signature;
    records_.resetPrefix(selfplay_prefix + "-" + std::to_string(ver_));
  }

  void feed(const Record& record) {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    const MsgResult& r = record.result;

    const bool didBlackWin = r.reward > 0;
    if (didBlackWin) {
      black_win_++;
    } else {
      white_win_++;
    }
    counter_++;
    records_.feed(record);

    if (r.num_move < 100)
      move0_100++;
    else if (r.num_move < 200)
      move100_200++;
    else if (r.num_move < 300)
      move200_300++;
    else
      move300_up++;

    if (counter_ - last_counter_shown_ >= 100) {
      logger_->info("{}\n{}", elf_utils::now(), info());
      last_counter_shown_ = counter_;
    }
  }

  int n() const {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    return counter_;
  }

  bool is_check_point() const {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    if (options_.selfplay_init_num > 0 && options_.selfplay_update_num > 0) {
      return (
          counter_ == options_.selfplay_init_num ||
          ((counter_ > options_.selfplay_init_num) &&
           (counter_ - options_.selfplay_init_num) %
                   options_.selfplay_update_num ==
               0));
    } else {
      // Otherwise just save one every 1000 games.
      return counter_ > 0 && counter_ % 1000 == 0;
    }
  }

  bool checkAndSave() {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    if (is_check_point()) {
      records_.saveCurrent();
      records_.clear();
      return true;
    } else {
      return false;
    }
  }

  bool needWaitForMoreSample() const {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    if (options_.selfplay_init_num <= 0)
      return false;
    if (counter_ < options_.selfplay_init_num)
      return true;

    if (options_.selfplay_update_num <= 0)
      return false;
    return counter_ < options_.selfplay_init_num +
        options_.selfplay_update_num * num_weight_update_;
  }

  void notifyWeightUpdate() {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    num_weight_update_++;
  }

  void fillInRequest(const ClientInfo&, MsgRequest* msg) const {
    display_debug_info("SelfPlayRecord", __FUNCTION__, RED_B);

    msg->client_ctrl.async = options_.selfplay_async;
  }

  std::string info() const {
    const int n = black_win_ + white_win_;
    const float black_win_rate = static_cast<float>(black_win_) / (n + 1e-10);

    std::stringstream ss;
    ss << "=== Record Stats (" << ver_ << ") ====" << std::endl;
    ss << "B/W/A: " << black_win_ << "/" << white_win_ << "/" << n << " ("
       << black_win_rate * 100 << "%). ";
    ss << "Move: [0, 100): " << move0_100 << ", [100, 200): " << move100_200
       << ", [200, 300): " << move200_300 << ", [300, up): " << move300_up
       << std::endl;
    ss << "=== End Record Stats ====" << std::endl;

    return ss.str();
  }

 private:
  // statistics.
  const int64_t ver_;
  const GameOptions& options_;

  RecordBuffer records_;

  int black_win_ = 0, white_win_ = 0;
  int move0_100 = 0, move100_200 = 0, move200_300 = 0, move300_up = 0;
  int counter_ = 0;
  int last_counter_shown_ = 0;
  int num_weight_update_ = 0;

  std::shared_ptr<spdlog::logger> logger_;
};




















class SelfPlaySubCtrl {
 public:
  enum CtrlResult {
    VERSION_OLD,
    VERSION_INVALID,
    INSUFFICIENT_SAMPLE,
    SUFFICIENT_SAMPLE
  };

  SelfPlaySubCtrl(const GameOptions& options, const TSOptions& mcts_options)
      : options_(options),
        mcts_options_(mcts_options),
        curr_ver_(-1),
        logger_(elf::logging::getLogger(
            "elfgames::go::train::SelfPlaySubCtrl-",
            "")) {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);
  }

  FeedResult feed(const Record& r) {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);

    if (!r.request.vers.is_selfplay())
      return NOT_SELFPLAY;
    if (curr_ver_ != r.request.vers.black_ver)
      return VERSION_MISMATCH;

    auto* perf = find_or_null(r.request.vers.black_ver);
    if (perf == nullptr)
      return NOT_REQUESTED;

    perf->feed(r);
    total_selfplay_++;
    if (total_selfplay_ % 1000 == 0) {
      logger_->info(
          "{} SelfPlaySubCtrl: # total selfplays processed by feed(): {}, {}",
          elf_utils::now(),
          total_selfplay_);
    }
    perf->checkAndSave();
    return FEEDED;
  }

  int64_t getCurrModel() const {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);
    return curr_ver_;
  }

  bool setCurrModel(int64_t ver) {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);
    if (ver != curr_ver_) {
      logger_->info("SelfPlay: {} -> {}", curr_ver_, ver);
      curr_ver_ = ver;
      find_or_create(curr_ver_);
      return true;
    }
    return false;
  }

  CtrlResult needWaitForMoreSample(int64_t selfplay_ver) const {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);
    if (selfplay_ver < curr_ver_)
      return VERSION_OLD;

    const auto* perf = find_or_null(curr_ver_);
    if (perf == nullptr)
      return VERSION_INVALID;
    return perf->needWaitForMoreSample() ? INSUFFICIENT_SAMPLE
                                         : SUFFICIENT_SAMPLE;
  }

  void notifyCurrentWeightUpdate() {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);

    auto* perf = find_or_null(curr_ver_);
    assert(perf != nullptr);
    return perf->notifyWeightUpdate();
  }

  int getNumSelfplayCurrModel() {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);
    auto* perf = find_or_null(curr_ver_);
    if (perf != nullptr)
      return perf->n();
    else
      return 0;
  }

  void fillInRequest(const ClientInfo& info, MsgRequest* msg) {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    std::lock_guard<std::mutex> lock(mutex_);

    if (curr_ver_ < 0) {
      msg->vers.set_wait();
    } else {
      auto* perf = find_or_null(curr_ver_);
      assert(perf != nullptr);
      msg->vers.black_ver = curr_ver_;
      msg->vers.white_ver = -1;
      msg->vers.mcts_opt = mcts_options_;
      perf->fillInRequest(info, msg);
    }
  }

 private:
  mutable std::mutex mutex_;

  GameOptions options_;
  TSOptions mcts_options_;
  int64_t curr_ver_;
  std::unordered_map<int64_t, std::unique_ptr<SelfPlayRecord>> perfs_;

  int64_t total_selfplay_ = 0;

  std::shared_ptr<spdlog::logger> logger_;

  SelfPlayRecord& find_or_create(int64_t ver) {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    auto it = perfs_.find(ver);
    if (it != perfs_.end()) {
      return *it->second;
    }
    auto* record = new SelfPlayRecord(ver, options_);
    perfs_[ver].reset(record);
    return *record;
  }

  SelfPlayRecord* find_or_null(int64_t ver) {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);

    auto it = perfs_.find(ver);
    if (it == perfs_.end()) {
      logger_->info("The version {} was not sent before!", std::to_string(ver));
      return nullptr;
    }
    return it->second.get();
  }

  const SelfPlayRecord* find_or_null(int64_t ver) const {
    display_debug_info("SelfPlaySubCtrl", __FUNCTION__, RED_B);
    
    auto it = perfs_.find(ver);
    if (it == perfs_.end()) {
      logger_->info("The version {} was not sent before!", std::to_string(ver));
      return nullptr;
    }
    return it->second.get();
  }
};
