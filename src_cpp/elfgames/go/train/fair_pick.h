/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "client_manager.h"

namespace fair_pick {

struct Info {
  enum Status { WAIT, SETTLED, STUCK };

  float r;
  Status status = WAIT;

  bool Add(float rr) {
    display_debug_info("struct Info", __FUNCTION__, RED_B);

    if (status == SETTLED)
      return false;
    r = rr;
    status = SETTLED;
    return true;
  }
};

enum WinCountEstimate { WIN, LOSS, INCOMPLETE };

struct WinCount {
 public:
  void Add(float r) {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    if (r > 0)
      n_win_++;
    n_done_++;
  }

  void SetNumStuck(int n_stuck) {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    n_stuck_ = n_stuck;
  }

  // Getters.
  bool IsDone(int n_request) const {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    return n_stuck_ + n_done_ == n_request;
  }

  int n_done() const {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    return n_done_;
  }

  int n_win() const {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    return n_win_;
  }

  int n_stuck() const {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    return n_stuck_;
  }

  float winrate() const {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    return n_done_ > 0 ? static_cast<float>(n_win_) / n_done_ : 0.0;
  }

  WinCountEstimate CheckWinrateBound(int n_request, float wr_thres) const {
    display_debug_info("struct WinCount", __FUNCTION__, RED_B);

    int n_done_max = n_request - n_stuck_;
    int n_uncertain = n_done_max - n_done_;
    float upper = static_cast<float>(n_uncertain + n_win_) / n_done_max;
    float lower = static_cast<float>(n_win_) / n_done_max;

    if (upper < wr_thres)
      return LOSS;
    if (lower >= wr_thres)
      return WIN;
    return INCOMPLETE;
  }

  WinCount& operator+=(const WinCount& wc) {
    n_stuck_ += wc.n_stuck_;
    n_done_ += wc.n_done_;
    n_win_ += wc.n_win_;
    return *this;
  }

  friend WinCount operator+(const WinCount& wc1, const WinCount& wc2) {
    WinCount wc = wc1;
    wc += wc2;
    return wc;
  }

  std::string info() const {
    if (n_done_ == 0)
      return std::string("No Game");

    std::stringstream ss;
    const float wr = static_cast<float>(n_win_) / n_done_;
    ss << "wr: " << wr << ", " << n_win_ << "/" << n_done_ - n_win_ << "/"
       << n_done_;

    /*
    if (! IsDone()) {
      const pair<float, float> bounds = winrate_bound();
      ss << ", wr_range: [" << bounds.first << ", " << bounds.second << "]";
    }
    */

    return ss.str();
  }

 private:
  // n_stuck + n_done == n_request
  int n_stuck_ = 0;
  int n_done_ = 0;
  int n_win_ = 0;
};













enum RegisterResult {
  NEWLY_REGISTERED,
  REGISTERED_WAITING,
  REGISTERED_SETTLED,
  AT_CAPACITY
};
enum AddResult { NOT_REGISTERED, NEWLY_ADDED, OVERFLOW_NOT_ADDED };

inline bool need_request(RegisterResult r) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  return r == NEWLY_REGISTERED || r == REGISTERED_WAITING;
}

inline bool release_request(RegisterResult r) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  return !need_request(r);
}
















class BatchRequest {
 public:
  using Key = std::string;

  BatchRequest(size_t max_num_request) : max_num_request_(max_num_request) {
    display_debug_info("BatchRequest", __FUNCTION__, RED_B);
  }

  RegisterResult Reg(const ClientInfo& c) {
    display_debug_info("BatchRequest", __FUNCTION__, RED_B);

    auto it = requests_.find(c.id());
    if (it == requests_.end()) {
      if (requests_.size() >= max_num_request_) {
        // We are at capacity and won't register anymore.
        return AT_CAPACITY;
      } else {
        requests_.insert(make_pair(c.id(), Info()));
        return NEWLY_REGISTERED;
      }
    } else {
      // Check its current status.
      if (it->second.status == Info::WAIT)
        return REGISTERED_WAITING;
      else
        return REGISTERED_SETTLED;
    }
  }

  AddResult Add(const ClientInfo& c, float r) {
    display_debug_info("BatchRequest", __FUNCTION__, RED_B);

    auto it = requests_.find(c.id());
    if (it == requests_.end()) {
      return NOT_REGISTERED;
    }
    if (!it->second.Add(r)) {
      return OVERFLOW_NOT_ADDED;
    }

    win_count_.Add(r);

    return NEWLY_ADDED;
  }

  void CheckStuck(const ClientManager& mgr) {
    display_debug_info("BatchRequest", __FUNCTION__, RED_B);

    auto curr_timestamp = mgr.getCurrTimeStamp();

    stucks_.clear();
    nonstuck_zero_.clear();
    for (auto& p : requests_) {
      if (p.second.status == Info::SETTLED)
        continue;

      uint64_t delay = 0;
      const ClientInfo* c = mgr.getClientC(p.first);

      if (c == nullptr || c->IsStuck(curr_timestamp, &delay)) {
        p.second.status = Info::STUCK;
        stucks_.push_back(p.first);
      } else if (p.second.status == Info::WAIT) {
        nonstuck_zero_[p.first] = delay;
      }
    }
    win_count_.SetNumStuck(stucks_.size());
  }

  const WinCount& win_count() const {
    display_debug_info("BatchRequest", __FUNCTION__, RED_B);

    return win_count_;
  }

  size_t n_reg() const {
    display_debug_info("BatchRequest", __FUNCTION__, RED_B);

    return requests_.size();
  }

  bool IsDone() const {
    display_debug_info("BatchRequest", __FUNCTION__, RED_B);

    // At least one request.
    if (requests_.empty())
      return false;
    return win_count_.IsDone((int)requests_.size());
  }

  std::string stuck_info() const {
    std::stringstream ss;
    if (stucks_.size() > 0) {
      ss << "#st: " << stucks_.size() << ", " << stucks_[0];
    }
    if (nonstuck_zero_.size() > 0) {
      auto it = nonstuck_zero_.begin();
      ss << ", #non_st_0: " << nonstuck_zero_.size() << ", " << it->first
         << ", dl: " << it->second;
    }
    return ss.str();
  }

  std::string stuck_detail_info() const {
    std::stringstream ss;
    ss << std::hex << "BatchRequest Addr: " << this << std::dec << "** ";
    if (stucks_.size() > 0) {
      ss << "#st: " << stucks_.size() << "[";
      for (const std::string& s : stucks_) {
        ss << s << ", ";
      }
      ss << "]";
    }
    if (nonstuck_zero_.size() > 0) {
      ss << ", #non_st_0: " << nonstuck_zero_.size() << "[";
      for (const auto& p : nonstuck_zero_) {
        ss << p.first << "(dl:" << p.second << "), ";
      }
      ss << "]";
    }
    return ss.str();
  }

 private:
  const size_t max_num_request_;

  std::unordered_map<Key, Info> requests_;
  std::vector<Key> stucks_;
  std::unordered_map<Key, uint64_t> nonstuck_zero_;

  WinCount win_count_;
};




















class Pick {
 public:
  Pick(size_t num_request, size_t max_request_per_layer)
      : num_request_(num_request),
        max_request_per_layer_(max_request_per_layer),
        remaining_request_((int)num_request) {
    display_debug_info("Pick", __FUNCTION__, RED_B);

    set_new_request();
  }

  RegisterResult reg(const ClientInfo& c) {
    display_debug_info("Pick", __FUNCTION__, RED_B);

    return request_->Reg(c);
  }

  // Simple rule: first register, then add.
  // Any results without registration will be discarded. This is because
  // these results may have potential bias.
  AddResult add(const ClientInfo& c, float r) {
    display_debug_info("Pick", __FUNCTION__, RED_B);

    return request_->Add(c, r);
  }

  void checkStuck(const ClientManager& cm) {
    display_debug_info("Pick", __FUNCTION__, RED_B);

    request_->CheckStuck(cm);

    // Check whether the layers are done.
    if (request_->IsDone()) {
      // Summarize the result and go to the next one.
      win_count_ += request_->win_count();
      remaining_request_ -= request_->win_count().n_done();
      set_new_request();
    }
  }

  int numFinishedLayer() const {
    display_debug_info("Pick", __FUNCTION__, RED_B);

    return num_finished_layer_;
  }

  const WinCount& win_count() const {
    display_debug_info("Pick", __FUNCTION__, RED_B);

    return win_count_;
  }

  int n_reg_to_go() const {
    display_debug_info("Pick", __FUNCTION__, RED_B);

    return remaining_request_ - request_->n_reg();
  }

  std::string info() const {
    std::stringstream ss;
    ss << "l_finished:" << num_finished_layer_
       << ",req: done:" << (num_request_ - remaining_request_)
       << "/tot:" << num_request_ << "/lmax:" << max_request_per_layer_;
    ss << ", " << win_count_.info();
    ss << ", last_wc: " << request_->win_count().info();
    ss << ", " << request_->stuck_info();
    return ss.str();
  }

 private:
  const size_t num_request_, max_request_per_layer_;
  int remaining_request_;

  std::unique_ptr<BatchRequest> request_;
  WinCount win_count_;
  int num_finished_layer_ = 0;

  void set_new_request() {
    display_debug_info("Pick", __FUNCTION__, RED_B);
    
    size_t new_request = remaining_request_ > 0
        ? std::min(max_request_per_layer_, (size_t)remaining_request_)
        : 0;
    request_.reset(new BatchRequest(new_request));
    if (new_request > 0)
      num_finished_layer_++;
  }
};

} // namespace fair_pick
