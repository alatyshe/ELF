#pragma once


#include <deque>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "elf/debug/debug.h"

#include "../base/common.h"
#include "CheckersBoard.h"
#include "CheckersFeature.h"

// Two special moves.
// Used when we want to skip and let the opponent play.


class CheckersState {
 public:
  CheckersState() {
    reset();
  }

  bool forward(const Coord& c);
  bool checkMove(const Coord& c) const;

  void setFinalValue(float final_value) {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");

    _final_value = final_value;
    _has_final_value = true;
  }

  float getFinalValue() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");

    return _final_value;
  }

  bool HasFinalValue() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");

    return _has_final_value;
  }

  void reset();

  CheckersState(const CheckersState& s)
      : _history(s._history),
        // _board_hash(s._board_hash),
        _moves(s._moves),
        _final_value(s._final_value),
        _has_final_value(s._has_final_value) {

    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    CheckersCopyBoard(&_board, &s._board);
  }

  const CheckersBoard& board() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");

    return _board;
  }

  // Note that ply started from 1.
  bool justStarted() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");

    return _board._ply == 1;
  }

  int getPly() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    
    return _board._ply;
  }

 //  bool isTwoPass() const {
 //    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
 //    return _board._last_move == M_PASS && _board._last_move2 == M_PASS;
 //  }

  bool terminated() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");

    return is_over(_board) || getPly() >= TOTAL_MAX_MOVE ;
    // Original
    // return isTwoPass() || getPly() >= TOTAL_MAX_MOVE || _check_superko();
  }

  int lastMove() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    return _board._last_move;
  }

 //  Coord lastMove2() const {
 //    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
 //    return _board._last_move2;
 //  }

  int nextPlayer() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    return _board.active;
  }

  bool moves_since(size_t* next_move_number, std::vector<Coord>* moves) const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    if (*next_move_number > _moves.size()) {
      // The move number is not right.
      return false;
    }
    moves->clear();
    for (size_t i = *next_move_number; i < _moves.size(); ++i) {
      moves->push_back(_moves[i]);
    }
    *next_move_number = _moves.size();
    return true;
  }

 //  uint64_t getHashCode() const {
 //    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
 //    return _board._hash;
 //  }

  const std::vector<Coord>& getAllMoves() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    return _moves;
  }

  std::string getAllMovesString() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    std::stringstream ss;
    for (const Coord& c : _moves) {
      ss << "[" << c << "] ";
    }
    return ss.str();
  }

  std::string showBoard() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    std::string   res;

    res = get_true_state_str(_board);
    return std::string(res) + "\nLast move\t: " + std::to_string(lastMove()) +
        "\nCurrentPlayer\t: " + (this->nextPlayer() == BLACK_PLAYER ? 
          "\x1b[6;32;40mBlack" : "\x1b[6;31;40mWhite") + 
        "\x1b[0m\nmove num\t: " + std::to_string(_board._ply) + "\n";
  }

  float evaluate() const {
    display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
    float final_score = 0.0;
    if (getPly() >= TOTAL_MAX_MOVE)
      final_score = -1;
    else
      final_score = this->nextPlayer() == BLACK_PLAYER ? -1.0 : 1.0;

    return final_score;
  }

  // TODO: not a good design..
  // const std::deque<BoardHistory>& getHistory() const {
  //   display_debug_info("CheckersState", __FUNCTION__, "\x1b[1;36;40m");
  //   return _history;
  // }

 protected:
  CheckersBoard       _board;
  std::deque<CheckersBoardHistory> _history;

 //  struct _BoardRecord {
 //    Board::Bits bits;
 //  };

 //  std::unordered_map<uint64_t, std::vector<_BoardRecord>> _board_hash;

  std::vector<Coord> _moves;
  float _final_value = 0.0;
  bool _has_final_value = false;

 //  void _add_board_hash(const Coord& c);
};



















