#pragma once

// elf
#include "elf/logging/IndexedLoggerFactory.h"
// checkers
#include "CheckersBoard.h"

// Do not used yet
struct CheckersBoardHistory {
  int   board[8][8];
  int   current_player;

  // Copy position of the pawns and kings on the board.
  CheckersBoardHistory(const CheckersBoard& b) {

    for (int y = 0; y < 8; y++) {
      for (int x = 0; x < 8; x++) {
          board[y][x] != b.board[y][x];
      }
    }
    current_player = b.current_player;
  }
};

class CheckersState;

class CheckersFeature {
 public:

  CheckersFeature(const CheckersState& s) 
      : s_(s) {
  }
  const CheckersState& state() const {
    return s_;
  }
  void extract(std::vector<float>* features) const;
  void extract(float* features) const;

 private:
  const CheckersState& s_;
  static constexpr int64_t kBoardRegion = CHECKERS_BOARD_SIZE * CHECKERS_BOARD_SIZE;

  // Compute features.
  void getPawns(int player, float* data) const;
  void getKings(int player, float* data) const;
  // void getHistory(int player, float* data) const;
};

/* 
  Sending info to the python side by this class 
  and wait until fields will be filled by python side.
*/
struct CheckersReply {
  const CheckersFeature& bf;
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

  CheckersReply(const CheckersFeature& bf) : bf(bf), pi(TOTAL_NUM_ACTIONS, 0.0) {
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
