#include "GameBoard.h"

#define myassert(p, text) \
  do {                    \
    if (!(p)) {           \
      printf((text));     \
    }                     \
  } while (0)

bool CompareBoards(GameBoard b1, GameBoard b2) {
	int res = 0;

	res += (b1.active != b2.active);
	res += (b1.passive != b2.passive);
	res += (b1.jump_action != b2.jump_action);
	res += (b1.pieces[BLACK_PLAYER] != b2.pieces[BLACK_PLAYER]);
	res += (b1.pieces[WHITE_PLAYER] != b2.pieces[WHITE_PLAYER]);
	res += (b1.white_win != b2.white_win);
	res += (b1.black_win != b2.black_win);
	res += (b1._last_move != b2._last_move);
	res += (b1._ply != b2._ply);

	return res == 0;
}


void ClearBoard(GameBoard* board) {
  
  board->active = BLACK_PLAYER;
  board->passive = WHITE_PLAYER;

  board->jump_action = 0;

  // board->active = WHITE_PLAYER;
  // board->passive = BLACK_PLAYER;
  board->pieces[BLACK_PLAYER] = 0x308460024000002;
  // board->pieces[WHITE_PLAYER] = 0x115180e;

  board->pieces[BLACK_PLAYER] = WHITE_BASE;
  board->pieces[WHITE_PLAYER] = BLACK_BASE;

  board->white_win = 0;
  board->black_win = 0;

  board->_last_move = M_INVALID;
  board->_ply = 1;
  
}


bool Play(GameBoard *board, int action_index) {
  /*
    Updates the game state to reflect the effects of the input
    move.

    A legal move is represented by an integer with exactly two
    bits turned on: the old position and the new position.
  */
  bool can_jump;
  
  uint64_t action;
  uint64_t jump;


  int active;
  int passive;
  int buffer;

  auto index = moves::i_to_m.find(action_index);
  action = index->second[0];
  jump = index->second[1] & 2;

  board->_ply++;
  if (action != -1) {
    if (jump != 0) {
      board->jump_action = action;
    }
    active = board->active;
    passive = board->passive;
    // Cвапаем биты на карте
    board->pieces[active] ^= action;



    if ((board->pieces[BLACK_PLAYER] & BLACK_BASE) == BLACK_BASE) {
      board->black_win++;
    } else if ((board->pieces[WHITE_PLAYER] & WHITE_BASE) == WHITE_BASE) {
      board->white_win++;
    } else {
      board->black_win = 0;
      board->white_win = 0;
    }


    if (jump) {
      // check next jump
      can_jump = (_jumps_from(*board, board->pieces[active], board->jump_action).size() != 0);
      if (can_jump) {
        return false;
      }
    }
  }

  board->jump_action = 0;
  buffer = board->active;
  board->active = board->passive;
  board->passive = buffer;
  return false;
}



std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(GameBoard board) {
  std::array<int, TOTAL_NUM_ACTIONS> result;

  std::vector<std::array<uint64_t, 2>> moves;  
  std::stringstream ss;

  result.fill(0);
  moves = get_legal_moves(board);

  for (auto i = moves.begin(); i != moves.end(); ++i) {
    ss << (*i)[0] << ", " << (*i)[1];
    // std::cout << ss.str() << " : |" << moves::m_to_i.find(ss.str())->second << "|" << std::endl;
    result[moves::m_to_i.find(ss.str())->second] = 1;
    ss.str("");
  }
  return result;
}


bool TryPlay(GameBoard board, Coord c) {
  uint64_t active_pieces;
  uint64_t all_pieces;
  uint64_t empty_pieces;

  uint64_t action;
  uint64_t direction;
  uint64_t jump_move; // jump

  auto index = moves::i_to_m.find(c);
  action = index->second[0];
  direction = index->second[1] & 1;
  jump_move = index->second[1] & 2;

  uint64_t pawn_pos;
  // check for leaving base both players

  all_pieces = board.pieces[BLACK_PLAYER] | board.pieces[WHITE_PLAYER];
  empty_pieces = 0xFFFFFFFFFFFFFFFF ^ all_pieces;

  if (board.jump_action != 0) {
    // pass action
    if (c == 416)
      return true;

    uint64_t old_pawn_place = board.jump_action ^ (all_pieces & board.jump_action);

    if ((board.jump_action & action) == 0)
      return false;
    if ((old_pawn_place & (board.jump_action & action)) != 0)
      return false;

    active_pieces = board.pieces[board.active];
    all_pieces = board.pieces[BLACK_PLAYER] | board.pieces[WHITE_PLAYER] | old_pawn_place;
    empty_pieces = 0xFFFFFFFFFFFFFFFF ^ all_pieces;
  } else if ((board.active == BLACK_PLAYER)
      && !(board.pieces[WHITE_PLAYER] & BLACK_BASE)
      && (board.pieces[BLACK_PLAYER] & WHITE_BASE)) {
    active_pieces = board.pieces[board.active] & WHITE_BASE;
  } else if ((board.active == WHITE_PLAYER)
      && !(board.pieces[BLACK_PLAYER] & WHITE_BASE)
      && (board.pieces[WHITE_PLAYER] & BLACK_BASE)) {
    active_pieces = board.pieces[board.active] & BLACK_BASE;
  } else {
    active_pieces = board.pieces[board.active];
  }

  pawn_pos = active_pieces & action;
  if (!pawn_pos) {
    return false;
  }
  if ((action ^ (pawn_pos & action)) == 0) {
    return false;
  }
  if (_ugolki_get_move_direction(action, active_pieces) != direction) {
    return false;
  }

  // Normal move
  if (!jump_move) {
    if (board.jump_action != 0)
      return false;
    // Check for empty cell.
    return empty_pieces & (action ^ pawn_pos);
  } else {
    // Jump
    int start_pos = log(pawn_pos) / log(2);
    int dest_pos = log(action ^ pawn_pos) / log(2);

    if (!(empty_pieces & (1UL << dest_pos))) {
      return false;
    }

    if (direction) {
      return all_pieces & (1UL << (start_pos + ((dest_pos - start_pos) / 2)));
    } else {
      return all_pieces & (1UL << (dest_pos + ((start_pos - dest_pos) / 2)));
    }
  }
  return false;

}

bool IsOver(GameBoard board) {
  if (board.black_win > 0 && board.white_win > 0) {
    return true;
  } else if (board.black_win == 2 || board.white_win == 2) {
    return true;
  }
  return false;
}

// translates the board in 8x8 format 
//      3: our kings 
//      1: our pawns 
//      -3: enemy kings 
//      -1: enemy pawns
std::array<std::array<int, 8>, 8> GetObservation(GameBoard board, int player) {
  std::array<std::array<int, 8>, 8> board_out;
  uint64_t bin_black_pawn;
  uint64_t bin_white_pawn;
  int x;
  int y;

  for (int i = 0; i < 8; i++) {
    board_out[i].fill(0);
  }

  bin_black_pawn = board.pieces[BLACK_PLAYER];
  bin_white_pawn = board.pieces[WHITE_PLAYER];

  if (player == BLACK_PLAYER){
    for (int i = 0; i < 64; i++) {
      if (((bin_black_pawn >> i) & 1) == 1) {
        x = i - i / 8 * 8;
        y = i / 8;
        board_out[y][x] = 1;
      } else if (((bin_white_pawn >> i) & 1) == 1) {
        x = i - i / 8 * 8;
        y = i / 8;
        board_out[y][x] = -1;
      }
    }
  } else {
    for (int i = 0; i < 64; i++) {
      if (((bin_black_pawn >> i) & 1) == 1) {
        x = i - i / 8 * 8;
        y = i / 8;
        board_out[7 - y][7 - x] = -1;
      } else if (((bin_white_pawn >> i) & 1) == 1) {
        x = i - i / 8 * 8;
        y = i / 8;
        board_out[7 - y][7 - x] = 1;
      }
    }
  }
  return (board_out);
}

std::array<std::array<int, 8>, 8> GetTrueObservation(GameBoard board) {
  return (GetObservation(board, BLACK_PLAYER));
}

// for display in terminal
std::string GetTrueObservationStr(const GameBoard board) {
  std::array<std::array<int, 8>, 8> observation = GetTrueObservation(board);
  std::stringstream res;
  std::stringstream coords;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      coords << std::setw(2) << std::right << (y * 8 + x);

      if (observation[y][x] == -1) {
        res << RED_C << " (" << coords.str() << ")P" << COLOR_END;
      } else if (observation[y][x] == 1) {
        res << GREEN_C << " (" << coords.str() << ")P" << COLOR_END;
      } else {
        res << " (" << coords.str() << ")E";
      }
      coords.str("");
    }
    res << "\n";
  }
  return(res.str());
}

void CopyBoard(GameBoard* dst, const GameBoard* src) {
  myassert(dst, "dst cannot be nullptr");
  myassert(src, "src cannot be nullptr");

  memcpy(dst, src, sizeof(GameBoard));
}







// just board logic
uint64_t _ugolki_right(GameBoard board, uint64_t pieces) {
  uint64_t invalid = 0x0101010101010101 >> 1;
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ (board.pieces[BLACK_PLAYER] | board.pieces[WHITE_PLAYER]);
  return (empty >> 1) & (pieces ^ (pieces & invalid));
}
uint64_t _ugolki_left(GameBoard board, uint64_t pieces) {
  uint64_t invalid = 0x0101010101010101;
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ (board.pieces[BLACK_PLAYER] | board.pieces[WHITE_PLAYER]);
  return (empty << 1) & (pieces ^ (pieces & invalid));
}
uint64_t _ugolki_forward(GameBoard board, uint64_t pieces) {
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ (board.pieces[BLACK_PLAYER] | board.pieces[WHITE_PLAYER]);
  return (empty >> 8) & (pieces);
}
uint64_t _ugolki_backward(GameBoard board, uint64_t pieces) {
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ (board.pieces[BLACK_PLAYER] | board.pieces[WHITE_PLAYER]);
  return (empty << 8) & (pieces);
}
uint64_t _ugolki_right_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move) {
  uint64_t invalid = 0x0303030303030303 >> 2;
  uint64_t all_pieces = board.pieces[WHITE_PLAYER] | board.pieces[BLACK_PLAYER] | invalid_move;
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ all_pieces;
  return ((empty >> 2) & (pieces ^ (pieces & invalid)) & (all_pieces >> 1));
}
uint64_t _ugolki_left_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move) {
  uint64_t invalid = 0x0303030303030303;
  uint64_t all_pieces = board.pieces[WHITE_PLAYER] | board.pieces[BLACK_PLAYER] | invalid_move;
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ all_pieces;
  return ((empty << 2) & (pieces ^ (pieces & invalid)) & (all_pieces << 1));
}
uint64_t _ugolki_forward_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move) {
  uint64_t all_pieces = board.pieces[WHITE_PLAYER] | board.pieces[BLACK_PLAYER] | invalid_move;
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ all_pieces;
  return ((empty >> 16) & pieces & (all_pieces >> 8));
}
uint64_t _ugolki_backward_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move) {
  uint64_t all_pieces = board.pieces[WHITE_PLAYER] | board.pieces[BLACK_PLAYER] | invalid_move;
  uint64_t empty = 0xFFFFFFFFFFFFFFFF ^ all_pieces;
  return ((empty << 16) & pieces & (all_pieces << 8));
}




uint64_t _ugolki_get_move_direction(uint64_t move, uint64_t pieces) {
  if (move < 0) {
    move = -move;
  }
  return (pieces < (pieces ^ move));
}


  // board->pieces[BLACK_PLAYER] = WHITE_BASE;
  // board->pieces[WHITE_PLAYER] = BLACK_BASE;
std::vector<std::array<uint64_t, 2>> get_legal_moves(GameBoard board) {
  int active = board.active;
  int passive = board.passive;

  // check for leaving base both players
  if (board.jump_action != 0) {
    return _jumps_from(board, board.pieces[active], board.jump_action);
  } else if ((active == BLACK_PLAYER)
      && !(board.pieces[WHITE_PLAYER] & BLACK_BASE)
      && (board.pieces[BLACK_PLAYER] & WHITE_BASE)) {
    return _get_all_moves(board, board.pieces[BLACK_PLAYER] & WHITE_BASE);
  } else if ((active == WHITE_PLAYER)
      && !(board.pieces[BLACK_PLAYER] & WHITE_BASE)
      && (board.pieces[WHITE_PLAYER] & BLACK_BASE)) {
    return _get_all_moves(board, board.pieces[WHITE_PLAYER] & BLACK_BASE);
  }
  return _get_all_moves(board, board.pieces[active]);
}


std::vector<std::array<uint64_t, 2>> _get_all_moves(GameBoard board, uint64_t pieces) {
  /*
    Returns a list of all possible moves.

    A legal move is represented by an integer with exactly two
    bits turned on: the old position and the new position.

    Jumps are indicated with a negative sign.
  */
  uint64_t rm;
  uint64_t lm;
  uint64_t fm;
  uint64_t bm;

  uint64_t rj;
  uint64_t lj;
  uint64_t fj;
  uint64_t bj;

  std::vector<std::array<uint64_t, 2>> moves;
  uint64_t buff;
  std::array<uint64_t, 2> move_buff;

  rm = _ugolki_right(board, pieces);
  lm = _ugolki_left(board, pieces);
  fm = _ugolki_forward(board, pieces);
  bm = _ugolki_backward(board, pieces);

  rj = _ugolki_right_jumps(board, pieces);
  lj = _ugolki_left_jumps(board, pieces);
  fj = _ugolki_forward_jumps(board, pieces);
  bj = _ugolki_backward_jumps(board, pieces);

  std::stringstream ss;
  if (rm != 0) {
    buff = rm;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x3UL << i), _ugolki_get_move_direction((0x3UL << i), pieces)};
        moves.push_back(move_buff);
      }
    }
  }
  if (lm != 0) {
    buff = lm;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x3UL << (i-1)), _ugolki_get_move_direction((0x3UL << (i-1)), pieces)};
        moves.push_back(move_buff);
      }
    }
  }
  if (fm != 0) {
    buff = fm;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x101UL << i), _ugolki_get_move_direction((0x101UL << i), pieces)};
        moves.push_back(move_buff);
      }
    }
  }
  if (bm != 0) {
    buff = bm;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x101UL << (i-8)), _ugolki_get_move_direction((0x101UL << (i-8)), pieces)};
        moves.push_back(move_buff);
      }
    }
  }


  if (rj != 0) {
    buff = rj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x5UL << i), (_ugolki_get_move_direction((0x5UL << i), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  if (lj != 0) {
    buff = lj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x5UL << (i-2)), (_ugolki_get_move_direction((0x5UL << (i-2)), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  if (fj != 0) {
    buff = fj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x10001UL << i), (_ugolki_get_move_direction((0x10001UL << i), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  if (bj != 0) {
    buff = bj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x10001UL << (i-16)), (_ugolki_get_move_direction((0x10001UL << (i-16)), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  return moves;
}


std::vector<std::array<uint64_t, 2>> _jumps_from(GameBoard board, uint64_t pieces, uint64_t jump_action) {
  /*
      Returns list of all possible jumps from the piece indicated.

      The argument piece should be of the form 2**n, where n + 1 is
      the square of the piece in question (using the internal numeric
      representaiton of the board).
  */
  uint64_t rj;
  uint64_t lj;
  uint64_t fj;
  uint64_t bj;
  
  std::vector<std::array<uint64_t, 2>> moves;
  uint64_t buff;
  std::array<uint64_t, 2> move_buff;
  uint64_t pawn_position;
  uint64_t invalid_move;

  pawn_position = pieces & jump_action;
  invalid_move = jump_action ^ pawn_position;

  rj = _ugolki_right_jumps(board, pawn_position, invalid_move);
  lj = _ugolki_left_jumps(board, pawn_position, invalid_move);
  fj = _ugolki_forward_jumps(board, pawn_position, invalid_move);
  bj = _ugolki_backward_jumps(board, pawn_position, invalid_move);

  if (rj != 0) {
    buff = rj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x5UL << i), (_ugolki_get_move_direction((0x5UL << i), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  if (lj != 0) {
    buff = lj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x5UL << (i-2)), (_ugolki_get_move_direction((0x5UL << (i-2)), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  if (fj != 0) {
    buff = fj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x10001UL << i), (_ugolki_get_move_direction((0x10001UL << i), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  if (bj != 0) {
    buff = bj;
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1) {
        move_buff = {(0x10001UL << (i-16)), (_ugolki_get_move_direction((0x10001UL << (i-16)), pieces) | 2U)};
        moves.push_back(move_buff);
      }
    }
  }
  if (bj | fj | lj | rj) {
    move_buff = {0, 0};
    moves.push_back(move_buff);
  }
  return moves;
}
