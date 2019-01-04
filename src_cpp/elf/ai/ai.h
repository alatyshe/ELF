/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <atomic>
#include <iostream>
#include <string>
#include <vector>

#include "elf/base/context.h"
#include "elf/utils/utils.h"
#include "elf/debug/debug.h"

namespace elf {
namespace ai {

template <typename S, typename A>
class AI_T {
 public:
  using Action = A;
  using State = S;

  AI_T() {
    display_debug_info("AI_T", __FUNCTION__, GREEN_B);

  }

  void setID(int id) {
    display_debug_info("AI_T", __FUNCTION__, GREEN_B);

    id_ = id;
    onSetID();
    // LOG(INFO) << "setID: " << id << std::endl;
  }

  int getID() const {
    display_debug_info("AI_T", __FUNCTION__, GREEN_B);

    return id_;
  }

  // Given the current state, perform action and send the action to _a;
  // Return false if this procedure fails.
  virtual bool act(const S&, A*) {
    display_debug_info("AI_T", __FUNCTION__, GREEN_B);

    return true;
  }

  virtual bool act_batch(
      const std::vector<const S*>& /*batch_s*/,
      const std::vector<A*>& /*batch_a*/) {
    display_debug_info("AI_T", __FUNCTION__, GREEN_B);

    return true;
  }

  // End the game
  virtual bool endGame(const S&) {
    display_debug_info("AI_T", __FUNCTION__, GREEN_B);

    return true;
  }

  virtual ~AI_T() {}

 protected:
  virtual void onSetID() {
    display_debug_info("AI_T", __FUNCTION__, GREEN_B);
  }

 private:
  int id_ = -1;
};














template <typename S, typename A>
class AIClientT : public AI_T<S, A> {
 public:
  using Action = A;
  using State = S;

  AIClientT(elf::GameClient* client, const std::vector<std::string>& targets)
      : client_(client), targets_(targets) {
      display_debug_info("AIClientT", __FUNCTION__, GREEN_B);
      
  }

  // Given the current state, perform action and send the action to _a;
  // Return false if this procedure fails.
  bool act(const S& s, A* a) override {
    display_debug_info("AIClientT", __FUNCTION__, GREEN_B);

    elf::FuncsWithState funcs_s = client_->BindStateToFunctions(targets_, &s);
    elf::FuncsWithState funcs_a = client_->BindStateToFunctions(targets_, a);
    // elf::FuncsWithState funcs =
    // elf::FuncsWithState::MergePkg(funcs_s, funcs_a);

    funcs_s.add(funcs_a);

    comm::ReplyStatus status = client_->sendWait(targets_, &funcs_s);
    return status == comm::ReplyStatus::SUCCESS ||
        status == comm::ReplyStatus::UNKNOWN;
  }

  bool act_batch(
      const std::vector<const S*>& batch_s,
      const std::vector<A*>& batch_a) override {
    display_debug_info("AIClientT", __FUNCTION__, GREEN_B);
    
    std::vector<elf::FuncsWithState> funcs_s =
        client_->BindStateToFunctions(targets_, batch_s);
    std::vector<elf::FuncsWithState> funcs_a =
        client_->BindStateToFunctions(targets_, batch_a);
    // elf::FuncsWithState funcs =
    // elf::FuncsWithState::MergePkg(funcs_s, funcs_a);
    //
    std::vector<elf::FuncsWithState*> ptr_funcs_s;
    for (size_t i = 0; i < funcs_s.size(); ++i) {
      funcs_s[i].add(funcs_a[i]);
      ptr_funcs_s.push_back(&funcs_s[i]);
    }
    
    comm::ReplyStatus status = client_->sendBatchWait(targets_, ptr_funcs_s);
    return status == comm::ReplyStatus::SUCCESS ||
        status == comm::ReplyStatus::UNKNOWN;
  }

 private:
  elf::GameClient* client_;
  std::vector<std::string> targets_;
};

} // namespace ai
} // namespace elf
