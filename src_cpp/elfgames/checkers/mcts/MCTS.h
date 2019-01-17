/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <iostream>

#include "elf/ai/tree_search/mcts.h"
#include "elf/logging/IndexedLoggerFactory.h"

#include "../mcts/AI.h"


// AlphaGo Zero uses Tromp-Taylor scoring 66 during MCTS simulations 
// and self-play training. This is because human scores 
// (Chinese, Japanese or Korean rules) are not well-defined
// if the game terminates before territorial boundaries are resolved.
// However, all tournament and evaluation games were scored
// using Chinese rules.


struct MCTSActorParams {
  std::string actor_name;
  uint64_t  seed = 0;
  // Required model version.
  // If -1, then there is no requirement on model version (any model response
  // can be used).
  int64_t   required_version = -1;

  std::string info() const {
    std::stringstream ss;
    ss  << "[name=" << actor_name << "][seed=" 
        << seed << "][requred_ver=" << required_version << "]";
    return ss.str();
  }
};











class CheckersMCTSActor {
 public:
  using Action  = Coord;
  using State   = CheckersState;
  using NodeResponse = elf::ai::tree_search::NodeResponseT<Coord>;

  enum PreEvalResult { EVAL_DONE, EVAL_NEED_NN };

  CheckersMCTSActor(elf::GameClient* client, const MCTSActorParams& params)
      : params_(params),
        rng_(params.seed),
        logger_(elf::logging::getIndexedLogger(
          std::string("\x1b[1;35;40m|++|\x1b[0m") + 
          "CheckersMCTSActor-", 
          "")) {
    ai_.reset(new CheckersAI(client, {params_.actor_name}));
  }

  std::string info() const {
    return params_.info();
  }

  void set_ostream(std::ostream* oo) {
    oo_ = oo;
  }

  void setRequiredVersion(int64_t ver) {
    params_.required_version = ver;
  }

  std::mt19937* rng() {
    return &rng_;
  }

  // batch evaluate.
  void evaluate(
      const std::vector<const CheckersState*>& states,
      std::vector<NodeResponse>* p_resps) {
    if (states.empty())
      return;

    if (oo_ != nullptr)
      *oo_ << "Evaluating batch state. #states: " << states.size() << std::endl;

    auto& resps = *p_resps;

    resps.resize(states.size());
    std::vector<CheckersFeature> sel_bfs;
    std::vector<size_t> sel_indices;

    for (size_t i = 0; i < states.size(); i++) {
      assert(states[i] != nullptr);
      PreEvalResult res = pre_evaluate(*states[i], &resps[i]);
      if (res == EVAL_NEED_NN) {
        sel_bfs.push_back(get_extractor(*states[i]));
        sel_indices.push_back(i);
      }
    }

    if (sel_bfs.empty())
      return;

    std::vector<CheckersReply> replies;
    for (size_t i = 0; i < sel_bfs.size(); ++i) {
      replies.emplace_back(sel_bfs[i]);
    }

    // Get all pointers.
    std::vector<CheckersReply*> p_replies;
    std::vector<const CheckersFeature*> p_bfs;

    for (size_t i = 0; i < sel_bfs.size(); ++i) {
      p_bfs.push_back(&sel_bfs[i]);
      p_replies.push_back(&replies[i]);
    }

    if (!ai_->act_batch(p_bfs, p_replies)) {
      logger_->info("act unsuccessful! ");
    } else {
      for (size_t i = 0; i < sel_indices.size(); i++) {
        post_nn_result(replies[i], &resps[sel_indices[i]]);
      }
    }
  }

  void evaluate(const CheckersState& s, NodeResponse* resp) {
    if (oo_ != nullptr)
      *oo_ << "Evaluating state at " << std::hex << &s << std::dec << std::endl;

    // if terminated(), get results, res = done
    // else res = EVAL_NEED_NN
    PreEvalResult res = pre_evaluate(s, resp);

    if (res == EVAL_NEED_NN) {
      CheckersFeature bf = get_extractor(s);
      // CheckersReply struct initialization
      // members containing:
      // Coord c, vector<float> pi, float v;
      CheckersReply reply(bf);

      // AI-Client will run a one-step neural network
      if (!ai_->act(bf, &reply)) {
        // This happens when the game is about to end,
        logger_->info("act unsuccessful! ");
      } else {
        // call pi2response()
        // action will be inv-transformed
        post_nn_result(reply, resp);
      }
    }

    if (oo_ != nullptr)
      *oo_ << "Finish evaluating state at " << std::hex << &s << std::dec
           << std::endl;
  }

  bool forward(CheckersState& s, Coord a) {
    return s.forward(a);
  }

  void setID(int id) {
    ai_->setID(id);
  }

  float reward(const CheckersState& /*s*/, float value) const {
    return value;
  }

 protected:
  MCTSActorParams params_;
  std::unique_ptr<CheckersAI> ai_;
  std::ostream* oo_ = nullptr;
  std::mt19937 rng_;

 private:
  std::shared_ptr<spdlog::logger> logger_;

  CheckersFeature get_extractor(const CheckersState& s) {
    return CheckersFeature(s);
  }

  PreEvalResult pre_evaluate(const CheckersState& s, NodeResponse* resp) {
    resp->q_flip = s.nextPlayer() == WHITE_PLAYER;

    if (s.terminated()) {
      if (oo_ != nullptr) {
        *oo_ << "Terminal state at " << s.getPly() << " Use TT evaluator"
             << std::endl;
        *oo_ << "Moves[" << s.getAllMoves().size()
             << "]: " << s.getAllMovesString() << std::endl;
        *oo_ << s.showBoard() << std::endl;
      }
      float final_value = s.evaluate();
      if (oo_ != nullptr)
        *oo_ << "Terminal state. Get raw score (no komi): " << final_value
             << std::endl;
      resp->value = final_value > 0 ? 1.0 : -1.0;
      // No further action.
      resp->pi.clear();
      return EVAL_DONE;
    } else {
      return EVAL_NEED_NN;
    }
  }

  void post_nn_result(const CheckersReply& reply, NodeResponse* resp) {
    if (params_.required_version >= 0 &&
        reply.version != params_.required_version) {
      const std::string msg = "model version " + std::to_string(reply.version) +
          " and required version " + std::to_string(params_.required_version) +
          " are not consistent";
      logger_->error(msg);
      throw std::runtime_error(msg);
    }

    if (oo_ != nullptr)
      *oo_ << "Got information from neural network" << std::endl;
    resp->value = reply.value;

    const CheckersState& s = reply.bf.state();
    pi2response(reply.bf, reply.pi, &resp->pi, oo_);
  }

  static void normalize(std::vector<std::pair<Coord, float>>* output_pi) {
    assert(output_pi != nullptr);
    float total_prob = 1e-10;

    for (const auto& p : *output_pi) {
      total_prob += p.second;
    }

    for (auto& p : *output_pi) {
      p.second /= total_prob;
    }
  }

  static void pi2response(
      const CheckersFeature& bf,
      const std::vector<float>& pi,
      std::vector<std::pair<Coord, float>>* output_pi,
      std::ostream* oo = nullptr) {
    const CheckersState& s = bf.state();

    if (oo != nullptr) {
      *oo << "In get_last_pi, #move returned " << pi.size() << std::endl;
      *oo << s.showBoard() << std::endl << std::endl;
    }

    output_pi->clear();

    // No action for terminated state.
    if (s.terminated()) {
      if (oo != nullptr)
        *oo << "Terminal state at " << s.getPly() << std::endl;
      return;
    }

    for (size_t i = 0; i < pi.size(); ++i) {
      // Inv random transform will be applied
      Coord m = i;
      if (oo != nullptr)
        *oo << "  Action " << i << " to Coord "
            << elf::ai::tree_search::ActionTrait<Coord>::to_string(m)
            << std::endl;

      output_pi->push_back(std::make_pair(m, pi[i]));
    }
    // // sorting..
    using data_type = std::pair<Coord, float>;

    if (oo != nullptr)
      *oo << "Before sorting" << std::endl;

    std::sort(
        output_pi->begin(),
        output_pi->end(),
        [](const data_type& d1, const data_type& d2) {
          return d1.second > d2.second;
        });

    if (oo != nullptr)
      *oo << "After sorting" << std::endl;

    std::vector<data_type> tmp;
    int i = 0;
    while (true) {
      if (i >= (int)output_pi->size())
        break;
      const data_type& v = output_pi->at(i);
      // Check whether this move is right.
      bool valid = s.checkMove(v.first);
      if (valid) {
        tmp.push_back(v);
      }

      if (oo != nullptr) {
        *oo << "Predict [" << i << "][" << v.first << "] " << v.second;
        if (valid)
          *oo << " added" << std::endl;
        else
          *oo << " invalid" << std::endl;
      }
      i++;
    }
    *output_pi = tmp;
    normalize(output_pi);
    if (oo != nullptr)
      *oo << "#Valid move: " << output_pi->size() << std::endl;
  }
};














namespace elf {
namespace ai {
namespace tree_search {

template <>
struct ActorTrait<CheckersMCTSActor> {
 public:
  static std::string to_string(const CheckersMCTSActor& a) {
    return a.info();
  }
};

} // namespace tree_search
} // namespace ai
} // namespace elf












class MCTSCheckersAI : public elf::ai::tree_search::MCTSAI_T<CheckersMCTSActor> {
 public:
  MCTSCheckersAI(
      const elf::ai::tree_search::TSOptions& options,
      std::function<CheckersMCTSActor*(int)> gen)
      : elf::ai::tree_search::MCTSAI_T<CheckersMCTSActor>(options, gen) {
  }

  float getValue() const {
    // Check if we need to resign.
    const auto& result = getLastResult();

    if (result.total_visits == 0)
      return result.root_value;
    else
      return result.best_edge_info.getQSA();
  }

  elf::ai::tree_search::MCTSPolicy<Coord> getMCTSPolicy() const {
    const auto& result = getLastResult();
    auto policy = result.mcts_policy;

    policy.normalize();
    return policy;
  }

  void setRequiredVersion(int64_t ver) {
    auto* engine = getEngine();
    assert(engine != nullptr);

    for (size_t i = 0; i < engine->getNumActors(); ++i) {
      engine->getActor(i).setRequiredVersion(ver);
    }
  }
};































