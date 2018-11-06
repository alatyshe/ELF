#include "CheckersState.h"

///////////// CheckersState ////////////////////
// bool CheckersState::forward(const Coord& c) {
//   display_debug_info("CheckersState", __FUNCTION__, RED_B);

//   if (c == M_INVALID) {
//     throw std::range_error("CheckersState::forward(): move is M_INVALID");
//   }
//   if (terminated())
//     return false;

//   GroupId4 ids;
//   if (!TryPlay2(&_board, c, &ids))
//     return false;

//   _add_board_hash(c);

//   Play(&_board, &ids);

//   _moves.push_back(c);
//   _history.emplace_back(_board);
//   if (_history.size() > MAX_NUM_AGZ_HISTORY)
//     _history.pop_front();
//   return true;
// }

// bool CheckersState::_check_superko() const {
//   display_debug_info("CheckersState", __FUNCTION__, RED_B);

//   // Check superko rule.
//   // need to check whether last move is pass or not.
//   if (lastMove() == M_PASS)
//     return false;

//   uint64_t key = _board._hash;
//   auto it = _board_hash.find(key);
//   if (it != _board_hash.end()) {
//     for (const auto& r : it->second) {
//       if (isBitsEqual(_board._bits, r.bits))
//         return true;
//     }
//   }
//   return false;
// }

// void CheckersState::_add_board_hash(const Coord& c) {
//   display_debug_info("CheckersState", __FUNCTION__, RED_B);

//   if (c == M_PASS)
//     return;

//   uint64_t key = _board._hash;
//   auto& r = _board_hash[key];
//   r.emplace_back();
//   copyBits(r.back().bits, _board._bits);
// }

// bool CheckersState::checkMove(const Coord& c) const {
//   display_debug_info("CheckersState", __FUNCTION__, RED_B);

//   GroupId4 ids;
//   if (c == M_INVALID)
//     return false;
//   return TryPlay2(&_board, c, &ids);
// }

// void CheckersState::applyHandicap(int handi) {
//   display_debug_info("CheckersState", __FUNCTION__, RED_B);

//   _handi_table.apply(handi, &_board);
// }

void CheckersState::reset() {
  display_debug_info("CheckersState", __FUNCTION__, RED_B);
  
  // clearBoard(&_board);
  // _moves.clear();
  // _board_hash.clear();
  // _history.clear();
  _final_value = 0.0;
  _has_final_value = false;
}

// HandicapTable CheckersState::_handi_table;
