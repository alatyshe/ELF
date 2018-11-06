#pragma once

#include "elf/debug/debug.h"
#include "CheckersBoard.h"

class CheckersState {
 public:
  CheckersState() {
    // reset();
  }
  
 //  bool forward(const Coord& c);
 //  bool checkMove(const Coord& c) const;

 //  void setFinalValue(float final_value) {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);

 //    _final_value = final_value;
 //    _has_final_value = true;
 //  }

 //  float getFinalValue() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _final_value;
 //  }

 //  bool HasFinalValue() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _has_final_value;
 //  }

  void reset();
 //  void applyHandicap(int handi);

 //  CheckersState(const CheckersState& s)
 //      : _history(s._history),
 //        _board_hash(s._board_hash),
 //        _moves(s._moves),
 //        _final_value(s._final_value),
 //        _has_final_value(s._has_final_value) {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    copyBoard(&_board, &s._board);
 //  }

 //  static HandicapTable& handi_table() {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _handi_table;
 //  }

 //  const Board& board() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board;
 //  }

 //  // Note that ply started from 1.
 //  bool justStarted() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board._ply == 1;
 //  }

 //  int getPly() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board._ply;
 //  }

 //  bool isTwoPass() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board._last_move == M_PASS && _board._last_move2 == M_PASS;
 //  }

 //  bool terminated() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return isTwoPass() || getPly() >= BOARD_MAX_MOVE || _check_superko();
 //  }

 //  Coord lastMove() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board._last_move;
 //  }

 //  Coord lastMove2() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board._last_move2;
 //  }

 //  Stone nextPlayer() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board._next_player;
 //  }

 //  bool moves_since(size_t* next_move_number, std::vector<Coord>* moves) const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    if (*next_move_number > _moves.size()) {
 //      // The move number is not right.
 //      return false;
 //    }
 //    moves->clear();
 //    for (size_t i = *next_move_number; i < _moves.size(); ++i) {
 //      moves->push_back(_moves[i]);
 //    }
 //    *next_move_number = _moves.size();
 //    return true;
 //  }

 //  uint64_t getHashCode() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _board._hash;
 //  }

 //  const std::vector<Coord>& getAllMoves() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _moves;
 //  }
 //  std::string getAllMovesString() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    std::stringstream ss;
 //    for (const Coord& c : _moves) {
 //      ss << "[" << coord2str2(c) << "] ";
 //    }
 //    return ss.str();
 //  }

 //  std::string showBoard() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    char buf[2000];
 //    showBoard2Buf(&_board, SHOW_LAST_MOVE, buf);
 //    return std::string(buf) + "\n" + "Last move: " + coord2str2(lastMove()) +
 //        ", nextPlayer: " + (nextPlayer() == S_BLACK ? "Black" : "White") + "\n";
 //  }

 //  float evaluate(float komi, std::ostream* oo = nullptr) const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    float final_score = 0.0;
 //    if (_check_superko()) {
 //      final_score = nextPlayer() == S_BLACK ? 1.0 : -1.0;
 //    } else {
 //      final_score = (float)simple_tt_scoring(_board, oo) - komi;
 //    }

 //    return final_score;
 //  }

 //  // TODO: not a good design..
 //  const std::deque<BoardHistory>& getHistory() const {
 //    display_debug_info("CheckersState", __FUNCTION__, RED_B);
 //    return _history;
 //  }

 protected:
  CheckersBoard _board;
 //  std::deque<BoardHistory> _history;

 //  struct _BoardRecord {
 //    Board::Bits bits;
 //  };

 //  std::unordered_map<uint64_t, std::vector<_BoardRecord>> _board_hash;

 //  std::vector<Coord> _moves;
  float _final_value = 0.0;
  bool _has_final_value = false;

 //  static HandicapTable _handi_table;

 //  bool _check_superko() const;
 //  void _add_board_hash(const Coord& c);
};




















// struct GoReply {
//   const BoardFeature& bf;
//   Coord c;
//   std::vector<float> pi;
//   float value = 0;
//   // Model version.
//   int64_t version = -1;

//   GoReply(const BoardFeature& bf) : bf(bf), pi(BOARD_NUM_ACTION, 0.0) {
//     display_debug_info("struct GoReply", __FUNCTION__, RED_B);
//   }
// };
