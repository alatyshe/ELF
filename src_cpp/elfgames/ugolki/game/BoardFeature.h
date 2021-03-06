#pragma once

// elf
#include "elf/logging/IndexedLoggerFactory.h"

#include "GameBoard.h"

// Do not used yet
struct GameBoardHistory {
  GameBoard board;

  // Copy position of the pawns and kings on the board.
  GameBoardHistory(const GameBoard& b) {
    CopyBoard(&board, &b);
  }
};

class GameState;

class BoardFeature {
 public:

  BoardFeature(const GameState& s) 
      : s_(s) {
  }
  const GameState& state() const {
    return s_;
  }
  void extract(std::vector<float>* features) const;
  void extract(float* features) const;

 private:
  const GameState& s_;
  static constexpr int64_t kBoardRegion = BOARD_SIZE * BOARD_SIZE;

  // Compute features.
  void getPawns(int player, float* data) const;
  void getKings(int player, float* data) const;
  // void getHistory(int player, float* data) const;
};

/* 
  Sending info to the python side by this class 
  and wait until fields will be filled by python side.
*/
struct BoardReply {
  const BoardFeature& bf;
  // Best action
  int c;
  // Policy prediction, representing the model’s priors 
  // on available moves given the current board situation.
  std::vector<float> pi;
  // Filled by value function, that representing the
  // probability of the current player winning.
  float value = 0;
  // Model version.
  int64_t version = -1;

  BoardReply(const BoardFeature& bf) : bf(bf), pi(TOTAL_NUM_ACTIONS, 0.0) {
  }
  
  std::string   info() {
    std::stringstream ss;

    ss  << "c : " << c << std::endl
        << "pi : " << std::endl;
    for (auto i = pi.begin(); i != pi.end(); ++i)
      ss << *i << ", ";
    ss  << std::endl
        << "value : " << value << std::endl
        << "version : " << version << std::endl;
    // ss  << bf.state().showBoard() << std::endl;
    return ss.str();
  }
};
