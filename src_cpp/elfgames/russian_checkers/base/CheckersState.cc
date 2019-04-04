#include "CheckersState.h"

///////////// CheckersState ////////////////////
bool CheckersState::forward(const Coord& c) {
  if (c == M_INVALID)
    throw std::range_error("CheckersState::forward(): move is M_INVALID");
  if (terminated() || c > TOTAL_NUM_ACTIONS)
    return false;
  if (!CheckersTryPlay(_board, c))
    return false;

  
  CheckersPlay(&_board, c);
  _moves.push_back(c);
  // _history.emplace_back(_board);
  // if (_history.size() > MAX_CHECKERS_HISTORY)
  //   _history.pop_front();
  return true;
}

bool CheckersState::checkMove(const Coord& c) const {
  return CheckersTryPlay(_board, c);
}

void CheckersState::reset() {  
  ClearBoard(&_board);
  _moves.clear();
  // _history.clear();
  _final_value = 0.0;
}

std::string CheckersState::showBoard() const {
  std::stringstream ss;

  ss  << GetTrueObservationStr(_board);
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
float CheckersState::evaluateGame() const {
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


bool CheckersState::moves_since(size_t* next_move_number, 
    std::vector<Coord>* moves) const {
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