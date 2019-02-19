#pragma once

#include "CheckersBoard.h"
#include "elf/logging/IndexedLoggerFactory.h"

// Do not used yet
struct CheckersBoardHistory {
  std::array<int64_t, 2>  forward;
  std::array<int64_t, 2>  backward;
  std::array<int64_t, 2>  pieces;
  int64_t                 empty;
  int                     active;

  // copy position of the pawns and kings on the board
  CheckersBoardHistory(const CheckersBoard& b) {
    forward[BLACK_PLAYER] = b.forward[BLACK_PLAYER];
    backward[BLACK_PLAYER] = b.backward[BLACK_PLAYER];
    pieces[BLACK_PLAYER] = (forward[BLACK_PLAYER]) | (backward[BLACK_PLAYER]);
    forward[WHITE_PLAYER] = b.forward[WHITE_PLAYER];
    backward[WHITE_PLAYER] = b.backward[WHITE_PLAYER];
    pieces[WHITE_PLAYER] = (forward[WHITE_PLAYER]) | (backward[WHITE_PLAYER]);
    
    empty = empty;
    active = active;
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
  const CheckersState&    s_;
  static constexpr int64_t kBoardRegion = CHECKERS_BOARD_SIZE * CHECKERS_BOARD_SIZE;

  // Compute features.
  void getPawns(int player, float* data) const;
  void getKings(int player, float* data) const;
  // void getHistory(int player, float* data) const;
};

// Sending info to the python side by this class 
// and wait until fields will be filled by python side
struct CheckersReply {
  const CheckersFeature&  bf;
  // Best action
  int                     c;
  // Policy prediction, representing the model’s priors 
  // on available moves given the current board situation
  std::vector<float>      pi;
  // Filled by value function, that representing the
  // probability of the current player winning
  float                   value = 0;
  // Model version.
  int64_t                 version = -1;

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
