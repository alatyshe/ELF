#pragma once

// game
#include "GameBoard.h"
#include "BoardFeature.h"

class GameState {
 public:

  GameState(int id)
      : _game_idx(id) {
    reset();
  }
  
  GameState(const GameState& s)
      : _history(s._history),
        _moves(s._moves),
        _final_value(s._final_value) {
    GameCopyBoard(&_board, &s._board);
  }

  bool forward(const Coord& c);
  bool checkMove(const Coord& c) const;
  bool moves_since(size_t* next_move_number, std::vector<Coord>* moves) const;
  std::string showBoard() const;
  float evaluateGame() const;
  void reset();

  void setFinalValue(float final_value) {
    _final_value = final_value;
  }

  float getFinalValue() const {
    return _final_value;
  }

  const GameBoard& board() const {
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

  int getGameIdx() const {
    return _game_idx;
  }

  const std::deque<GameBoard>& getHistory() const {
    return _history;
  }

  // delete!!!!!!
  std::array<std::array<int, 8>, 8> getBoard() const {
    return GetTrueState(_board);
  }

 protected:
  GameBoard _board;
  int _game_idx;

  // History of states
  std::deque<GameBoard> _history;
  // history of moves for current board
  std::vector<Coord> _moves;

  float _final_value = 0.0;
};
