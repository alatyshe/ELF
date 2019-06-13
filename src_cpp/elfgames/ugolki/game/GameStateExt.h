#pragma once

#include <set>

// elf
#include "elf/ai/tree_search/tree_search_base.h"
#include "elf/logging/IndexedLoggerFactory.h"

#include "GameState.h"
#include "BoardFeature.h"
#include "GameOptions.h"
#include "../common/record.h"
#include "../sgf/sgf.h"
#include "Record.h"

enum FinishReason {
  MAX_STEP = 0,
  BOTH_REACHED_BASE,
  BLACK_WIN,
  WHITE_WIN,
};

/* 
  Client Side
  calls from start_client.sh
  Generates batches for server
*/
struct GameStateExt {
 public:
  GameStateExt(int game_idx, const GameOptions& game_options);

  void setRequest(const MsgRequest& request);
  void addCurrentModel();
  const MsgRequest& currRequest() const;
  Coord lastMove() const;
  void restart();
  ThreadState getThreadState() const;
  float getLastGameFinalValue() const;
  void showFinishInfo() const;
  bool forward(Coord c);
  int seq() const;
  const GameState& state() const;
  const GameOptions& gameOptions() const;
  void saveCurrentTree(const std::string& tree_info) const;
  void setFinalValue();

  // packing the result of the game in json for sending to the server
  GameRecord dumpRecord() const {
    GameRecord r;

    r.timestamp = elf_utils::sec_since_epoch_from_now();
    r.thread_id = _game_idx;
    r.seq = _seq;
    r.request = _curr_request;

    r.result.reward = _state.getFinalValue();
    r.result.content = coords2str(_state.getAllMoves());
    r.result.using_models =
        std::vector<int64_t>(_using_models.begin(), _using_models.end());
    r.result.policies = _mcts_policies;
    r.result.num_move = _state.getPly() - 1;
    r.result.values = _predicted_values;
    _logger->info("Dump Record:{}\n", r.info());
    return r;
  }

  // need to check it
  void addMCTSPolicy(
      const elf::ai::tree_search::MCTSPolicy<Coord>& mcts_policy) {
    const auto& policy = mcts_policy.policy;

    // First find the max value
    float max_val = 0.0;
    for (size_t k = 0; k < policy.size(); k++) {
      const auto& entry = policy[k];
      max_val = std::max(max_val, entry.second);
    }
    _mcts_policies.emplace_back();
    std::fill(
        _mcts_policies.back().prob,
        _mcts_policies.back().prob + TOTAL_NUM_ACTIONS,
        0);
    for (size_t k = 0; k < policy.size(); k++) {
      const auto& entry = policy[k];
      unsigned char c =
          static_cast<unsigned char>(entry.second / max_val * 255);
      _mcts_policies.back().prob[entry.first] = c;
    }
  }

  void addPredictedValue(float predicted_value) {
    _predicted_values.push_back(predicted_value);
  }

 protected:
  const int _game_idx;

  int _seq = 0;
  int _last_move_for_the_game;
  float _last_value;
  std::set<int64_t> _using_models;

  GameState _state;
  
  MsgRequest _curr_request;
  GameOptions _game_options;

  std::vector<GameCoordRecord> _mcts_policies;
  // board value
  std::vector<float> _predicted_values;

  std::shared_ptr<spdlog::logger> _logger;
};

/* 
  Server Side
  calls from start_server.sh
*/
class GameStateExtOffline {
 public:
  friend class GameFeature;

  GameStateExtOffline(int game_idx, const GameOptions& game_options)
      : _game_idx(game_idx),
        _bf(_state),
        _game_options(game_options),
        _logger(elf::logging::getIndexedLogger(
            MAGENTA_B + std::string("|++|") + COLOR_END + 
            "GameStateExtOffline-",
            "")) {
  }

  void fromRecord(const GameRecord& r) {
    _offline_all_moves = str2coords(r.result.content);
    _offline_winner = r.result.reward > 0 ? 1.0 : -1.0;

    _mcts_policies = r.result.policies;
    _curr_request = r.request;
    _seq = r.seq;
    _predicted_values = r.result.values;
    _state.reset();

    // std::cout << "GoStateExtOffline::fromRecord" << std::endl;
    // std::cout << r.result.content << std::endl;
    // std::cout << "Moves" << std::endl;
    // for (const Coord& c : _offline_all_moves) {
    //   std::cout << "[" << c << "] ";
    // }
    // std::cout << std::endl;
    // std::cout << "r.info()" << std::endl;
    // std::cout << r.info() << std::endl << std::endl;
    // std::cout << "=============================" << std::endl;
  }

  bool switchRandomMove(std::mt19937* rng) {
    // Random sample one move
    if ((int)_offline_all_moves.size() <= _game_options.num_future_actions - 1) {
      _logger->warn(
          "[{}] #moves {} smaller than {} - 1",
          _game_idx,
          _offline_all_moves.size(),
          _game_options.num_future_actions);
      exit(0);
      return false;
    }
    size_t move_to = (*rng)() %
        (_offline_all_moves.size() - _game_options.num_future_actions + 1);
    switchBeforeMove(move_to);
    return true;
  }

  void switchBeforeMove(size_t move_to) {
    assert(move_to < _offline_all_moves.size());

    _state.reset();
    for (size_t i = 0; i < move_to; ++i) {
      _state.forward(_offline_all_moves[i]);
    }
  }

  int getNumMoves() const {
    return _offline_all_moves.size();
  }

  float getPredictedValue(int move_idx) const {
    return _predicted_values[move_idx];
  }

 private:
  const int _game_idx;
  GameState _state;
  BoardFeature _bf;
  GameOptions _game_options;

  int _seq;
  MsgRequest _curr_request;

  std::vector<Coord> _offline_all_moves;
  float _offline_winner;

  std::vector<GameCoordRecord> _mcts_policies;
  std::vector<float> _predicted_values;

  std::shared_ptr<spdlog::logger> _logger;
};
