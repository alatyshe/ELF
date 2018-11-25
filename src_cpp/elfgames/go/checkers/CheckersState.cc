#include "CheckersState.h"

///////////// CheckersState ////////////////////
bool CheckersState::forward(const Coord& c) {
  display_debug_info("CheckersState", __FUNCTION__, "\x1b[2;30;43m");

  if (c == M_INVALID)
    throw std::range_error("CheckersState::forward(): move is M_INVALID");

  if (terminated())
  {
    std::cout << "terminated()" << std::endl; 
    return false;
  }

  if (!CheckersTryPlay(_board, c))
    return false;

  // _add_board_hash(c);

  CheckersPlay(&_board, c);

  _moves.push_back(c);
  // _history.emplace_back(_board);
  if (_history.size() > MAX_CHECKERS_HISTORY)
    _history.pop_front();
  return true;
}

// void CheckersState::_add_board_hash(const Coord& c) {
//   display_debug_info("CheckersState", __FUNCTION__, "\x1b[2;30;43m");

//   if (c == M_PASS)
//     return;

//   uint64_t key = _board._hash;
//   auto& r = _board_hash[key];
//   r.emplace_back();
//   copyBits(r.back().bits, _board._bits);
// }

// bool CheckersState::checkMove(const Coord& c) const {
//   display_debug_info("CheckersState", __FUNCTION__, "\x1b[2;30;43m");

//   GroupId4 ids;
//   if (c == M_INVALID)
//     return false;
//   return TryPlay2(&_board, c, &ids);
// }

void CheckersState::reset() {
  display_debug_info("CheckersState", __FUNCTION__, "\x1b[2;30;43m");
  
  ClearBoard(&_board);
  _moves.clear();
  // _board_hash.clear();
  // _history.clear();
  _final_value = 0.0;
  _has_final_value = false;
}
