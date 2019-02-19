#pragma once

// ELF
#include "elf/debug/debug.h"

// Checkers
#include "CheckersBoard.h"
#include "CheckersFeature.h"

class CheckersState {
 public:

  CheckersState() {
    reset();
  }
  
  CheckersState(const CheckersState& s)
      : _history(s._history),
        _moves(s._moves),
        _final_value(s._final_value),
        _has_final_value(s._has_final_value) {
    CheckersCopyBoard(&_board, &s._board);
  }

  bool forward(const Coord& c);
  bool checkMove(const Coord& c) const;

  void setFinalValue(float final_value) {
    _final_value = final_value;
    _has_final_value = true;
  }

  float getFinalValue() const {
    return _final_value;
  }

  bool HasFinalValue() const {
    return _has_final_value;
  }

  void reset();

  const CheckersBoard& board() const {
    return _board;
  }

  // Note that ply started from 1.
  bool justStarted() const {
    return _board._ply == 1;
  }

  // move number
  int getPly() const {
    return _board._ply;
  }

  bool terminated() const {
    return CheckersIsOver(_board) || getPly() >= TOTAL_MAX_MOVE ;
  }

  int lastMove() const {
    return _board._last_move;
  }

  int currentPlayer() const {
    return _board.active;
  }

  bool moves_since(size_t* next_move_number, std::vector<Coord>* moves) const {
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

  // Moves history in vector
  const std::vector<Coord>& getAllMoves() const {
    return _moves;
  }

  // Moves history in string
  std::string getAllMovesString() const {
    std::stringstream ss;
    for (const Coord& c : _moves) {
      ss << "[" << c << "] ";
    }
    return ss.str();
  }

  std::string showBoard() const {
    std::stringstream ss;

    ss  << GetTrueStateStr(_board);
    ss  << "[" << _board.forward[0]
        << "][" << _board.forward[1]
        << "][" << _board.backward[0]
        << "][" << _board.backward[1]
        << "][" << _board.pieces[0]
        << "][" << _board.pieces[1]
        << "][" << _board.empty << "]"
        << std::endl;

    if (lastMove() != M_INVALID)
      ss  << "\nLast move\t: " << moves::m_to_h.find(lastMove())->second;
    else
      ss  << "\nLast move\t: Invalid";

    ss  << "\nCurrentPlayer\t: ";
    if (this->currentPlayer() == BLACK_PLAYER)
      ss << GREEN_C << "Black" << COLOR_END;
    else
      ss << RED_C << "White" << COLOR_END;
    ss << "\nmove num\t: " << _board._ply << "\n";
    return ss.str();
  }

  // Eval game call on each node in tree.
  // Should return 0 if state is not terminate,
  // because we eval current state by value func on each step
  float evaluateGame() const {
    float final_score = 0.0;

    if (terminated()) {
      if (getPly() >= TOTAL_MAX_MOVE)
        final_score = -1;
      else if (this->currentPlayer() == BLACK_PLAYER)
        final_score = -1;
      else
        final_score = 1;
    }
    return final_score;
  }

  // const std::deque<BoardHistory>& getHistory() const {
  //   return _history;
  // }

 protected:
  CheckersBoard _board;

  // History for our net(dont use now)
  std::deque<CheckersBoardHistory> _history;
  // history of moves for current board
  std::vector<Coord> _moves;

  float _final_value = 0.0;
  bool _has_final_value = false;
};
