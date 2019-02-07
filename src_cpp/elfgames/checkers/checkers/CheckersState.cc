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
  _has_final_value = false;
}







