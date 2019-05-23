#include "GameBoard.h"

#define myassert(p, text) \
  do {                    \
    if (!(p)) {           \
      printf((text));     \
    }                     \
  } while (0)

void ClearBoard(GameBoard* board) {
  board->active = BLACK_PLAYER;
  board->passive = WHITE_PLAYER;

  board->empty = 0;
  board->_ply = 1;

  board->forward[BLACK_PLAYER] = 0x1eff;
  board->backward[BLACK_PLAYER] = 0;
  board->pieces[BLACK_PLAYER] = (board->forward[BLACK_PLAYER]) | (board->backward[BLACK_PLAYER]);

  board->forward[WHITE_PLAYER] = 0;
  board->backward[WHITE_PLAYER] = 0x7fbc00000;
  board->pieces[WHITE_PLAYER] = (board->forward[WHITE_PLAYER]) | (board->backward[WHITE_PLAYER]);

  board->_last_move = M_INVALID;
  board->empty = UNUSED_BITS ^ MASK ^ (board->pieces[BLACK_PLAYER] | board->pieces[WHITE_PLAYER]);
  board->jump = 0;

  std::fill(std::begin(board->_last_move_black), 
    std::end(board->_last_move_black), -1);
  std::fill(std::begin(board->_last_move_white), 
    std::end(board->_last_move_white), -1);
  board->_black_repeats_step = 0;
  board->_white_repeats_step = 0;
}

bool CheckersPlay(GameBoard *board, int64_t action_index) {
  /*
    Updates the game state to reflect the effects of the input
    move.

    A legal move is represented by an integer with exactly two
    bits turned on: the old position and the new position.
  */
  int64_t move;
  int64_t active;
  int64_t passive;
  int64_t taken_piece;
  int64_t destination;
  uint64_t buff;
  int buffer;

  auto index = moves::i_to_m.find(action_index);
  move = index->second[0];

  board->_last_move = action_index;
  active = board->active;

  // Check repeated moves.
  if (active == WHITE_PLAYER) {
    if (board->_last_move_white[1] == action_index) {
      board->_white_repeats_step += 1;
    } else {
      board->_white_repeats_step = 0;
      board->_remove_step_white = false;
    }
    board->_last_move_white[1] = board->_last_move_white[0];
    board->_last_move_white[0] = action_index;
    ;
  } else {
    if (board->_last_move_black[1] == action_index) {
      board->_black_repeats_step += 1;
    } else {
      board->_black_repeats_step = 0;
      board->_remove_step_black = false;
    }
    board->_last_move_black[1] = board->_last_move_black[0];
    board->_last_move_black[0] = action_index;
  }
  // End checking.

  passive = board->passive;
  buffer = 0;
  board->_ply += 1;
  if (move < 0) {
    move *= -1;

    buff = static_cast<uint64_t>(move);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        buffer += i;
    }
    taken_piece = 1 << buffer / 2;
    board->pieces[passive] ^= taken_piece;
    if (board->forward[passive] & taken_piece)
      board->forward[passive] ^= taken_piece;
    if (board->backward[passive] & taken_piece)
      board->backward[passive] ^= taken_piece;
    board->jump = 1;
  }

  board->pieces[active] ^= move;
  if (board->forward[active] & move)
    board->forward[active] ^= move;
  if (board->backward[active] & move)
    board->backward[active] ^= move;

  destination = move & board->pieces[active];
  board->empty = UNUSED_BITS ^ MASK ^ (board->pieces[BLACK_PLAYER] | board->pieces[WHITE_PLAYER]);

  if (board->jump) {
    // board->mandatory_jumps = _jumps_from(*board, destination);
    // if (board->mandatory_jumps.size() != 0)
    if (_jumps_from(*board, destination).size() != 0)
      return true;
  }

  if (active == BLACK_PLAYER && (destination & 0x780000000) != 0)
    board->backward[BLACK_PLAYER] |= destination;
  else if (active == WHITE_PLAYER && (destination & 0xf) != 0)
    board->forward[WHITE_PLAYER] |= destination;

  board->jump = 0;
  buffer = board->active;
  board->active = board->passive;
  board->passive = buffer;
  // std::cout << GetTrueStateStr(*board) << std::endl;
  return false;
}

std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(GameBoard board) {
  std::array<int, TOTAL_NUM_ACTIONS> result;
  std::vector<int64_t> moves;
  std::string move_buff;
  int buffer;
  int total_moves = 0;

  result.fill(0);

  moves = _get_moves(board);

  for (auto i = moves.begin(); i != moves.end(); ++i) {
    move_buff = std::to_string(*i) + ", "  + std::to_string(_get_move_direction(board, *i, board.active));
    // print moves
    // std::cout << move_buff << " : |" << moves::m_to_i.find(move_buff)->second << "|" << std::endl;
    result[moves::m_to_i.find(move_buff)->second] = 1;
    total_moves += 1;
  }

  // Repeat moves
  if (total_moves > 1
      && board.active == WHITE_PLAYER 
      && board._white_repeats_step >= REPEAT_MOVE) {
    result[board._last_move_white[1]] = 0;
  } else if (total_moves > 1
      && board.active == BLACK_PLAYER
      && board._black_repeats_step >= REPEAT_MOVE) {
    result[board._last_move_black[1]] = 0;
  }

  return result;
}

std::vector<std::array<int64_t, 2>> GetValidMovesNumberAndDirection(GameBoard board, int player) {
  std::vector<std::array<int64_t, 2>> result;
  std::array<int64_t, 2> move_buff;
  std::vector<int64_t> moves;
  int buffer;

  if (player != board.active) {
    buffer = board.active;
    board.active = board.passive;
    board.passive = buffer;

    moves = _get_moves(board);
    for (auto i = moves.begin(); i != moves.end(); ++i) {
      move_buff = {*i, _get_move_direction(board, *i, board.active)};
      result.push_back(move_buff);
    }

    buffer = board.active;
    board.active = board.passive;
    board.passive = buffer;
  } else {
    moves = _get_moves(board);
    for (auto i = moves.begin(); i != moves.end(); ++i) {
      move_buff = {*i, _get_move_direction(board, *i, board.active)};
      result.push_back(move_buff);
    }
  }
  return result;
}

bool CheckersTryPlay(GameBoard board, Coord c) {
  std::array<int, TOTAL_NUM_ACTIONS> res = GetValidMovesBinary(board);
  if (res[c])
    return true;
  return false;
}

bool CheckersIsOver(GameBoard board) {
  return (_get_moves(board).size() == 0);
}

// translates the board in 8x8 format 
//      3: our kings 
//      1: our pawns 
//      -3: enemy kings 
//      -1: enemy pawns
std::array<std::array<int, 8>, 8> GetObservation(GameBoard board, int player) {
  std::array<std::array<int, 8>, 8> board_out;
  int64_t bin_black_pawn;
  int64_t bin_black_king;
  int64_t bin_white_pawn;
  int64_t bin_white_king;
  int buff;
  int x;
  int y;

  for (int i = 0; i < 8; i++)
    board_out[i].fill(0);
    bin_black_pawn = board.forward[BLACK_PLAYER];
    bin_black_king = board.backward[BLACK_PLAYER];
    bin_white_pawn = board.backward[WHITE_PLAYER];
    bin_white_king = board.forward[WHITE_PLAYER];

    if (player == BLACK_PLAYER){
      for (int i = 0; i < 35; i++) {
        if (((bin_black_king >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[y][x] = 3;
        } else if (((bin_white_king >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[y][x] = -3;
        } else if (((bin_black_pawn >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[y][x] = 1;
        } else if (((bin_white_pawn >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[y][x] = -1;
        }
      }
    } else {
      for (int i = 0; i < 35; i++) {
        if (((bin_black_king >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[7 - y][7 - x] = -3;
        } else if (((bin_white_king >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[7 - y][7 - x] = 3;
        } else if (((bin_black_pawn >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[7 - y][7 - x] = -1;
        } else if (((bin_white_pawn >> i) & 1) == 1) {
          buff = (1+i-i/9)-1;
          x = 6-(buff)%4*2+((buff)/4)%2;
          y = 7-(buff)/4;
          board_out[7 - y][7 - x] = 1;
        }
      }
    }
  return (board_out);
}

std::array<std::array<int, 8>, 8> GetTrueState(GameBoard board) {
  return (GetObservation(board, BLACK_PLAYER));
}

// for display in terminal
std::string GetTrueStateStr(const GameBoard board) {
  std::array<std::array<int, 8>, 8> observation = GetTrueState(board);
  std::string str = "";
  std::string buff = "";
  std::stringstream coords;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      coords << std::setw(2) << std::right << std::to_string(y * 8 + x);

      if (observation[y][x] == -1) {
        buff = std::string(RED_C) + " (" + coords.str() + ")M" + COLOR_END;
      } else if (observation[y][x] == -3) {
        buff = std::string(RED_C) + " (" + coords.str() + ")K" + COLOR_END;
      } else if (observation[y][x] == 1) {
        buff = std::string(GREEN_C) + " (" + coords.str() + ")M" + COLOR_END;
      } else if (observation[y][x] == 3) {
        buff = std::string(GREEN_C) + " (" + coords.str() + ")K" + COLOR_END;
      } else {
        buff = " (" + coords.str() + ")E";
      }
      coords.str("");
      str = str + buff;
    }
    str = str + "\n";
  }
  return(str);
}

void GameCopyBoard(GameBoard* dst, const GameBoard* src) {
  myassert(dst, "dst cannot be nullptr");
  myassert(src, "src cannot be nullptr");

  memcpy(dst, src, sizeof(GameBoard));
}

// just board logic
int64_t _right_forward(GameBoard board) {
  return ((board.empty >> 4) & board.forward[board.active]);
}

int64_t _left_forward(GameBoard board) {
  return ((board.empty >> 5) & board.forward[board.active]);
}

int64_t _right_backward(GameBoard board) {
  return ((board.empty << 4) & board.backward[board.active]);
}

int64_t _left_backward(GameBoard board) {
  return ((board.empty << 5) & board.backward[board.active]);
}

int64_t _right_forward_jumps(GameBoard board) {
  return ((board.empty >> 8) & (board.pieces[board.passive] >> 4) & board.forward[board.active]);
}

int64_t _left_forward_jumps(GameBoard board) {
  return ((board.empty >> 10) & (board.pieces[board.passive] >> 5) & board.forward[board.active]);
}

int64_t _right_backward_jumps(GameBoard board) {
  return ((board.empty << 8) & (board.pieces[board.passive] << 4) & board.backward[board.active]);
}

int64_t _left_backward_jumps(GameBoard board) {
  return ((board.empty << 10) & (board.pieces[board.passive] << 5) & board.backward[board.active]);
}

int64_t _get_move_direction(GameBoard board, int64_t move, int player) {
  if (move < 0)
    move = -move;
  return (board.pieces[player] < (board.pieces[player] ^ move));
}

std::vector<int64_t> _get_moves(GameBoard board) {
  /*
    Returns a list of all possible moves.

    A legal move is represented by an integer with exactly two
    bits turned on: the old position and the new position.

    Jumps are indicated with a negative sign.
  */
  int64_t rf;
  int64_t lf;
  int64_t rb;
  int64_t lb;
  std::vector<int64_t> jumps;
  std::vector<int64_t> moves;
  uint64_t buff;
  // First check if we are in a jump sequence
  // if (board.jump) {
  //  return (board.mandatory_jumps);
  // }
  // Next check if there are jumps
  jumps = _get_jumps(board);
  if (jumps.size() != 0) {
    return (jumps);
  } else {
    rf = _right_forward(board);
    lf = _left_forward(board);
    rb = _right_backward(board);
    lb = _left_backward(board);

    buff = static_cast<uint64_t>(rf);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((static_cast<int64_t>(0x11) << i));
    }

    buff = static_cast<uint64_t>(lf);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((static_cast<int64_t>(0x21) << i));
    }

    buff = static_cast<uint64_t>(rb);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((static_cast<int64_t>(0x11) << (i - 4)));
    }

    buff = static_cast<uint64_t>(lb);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((static_cast<int64_t>(0x21) << (i - 5)));
    }
    return (moves);
  }
}

std::vector<int64_t> _get_jumps(GameBoard board) {
  /*
    Returns a list of all possible jumps.

    A legal move is represented by an integer with exactly two
    bits turned on: the old position and the new position.

    Jumps are indicated with a negative sign.
  */
  int64_t rfj;
  int64_t lfj;
  int64_t rbj;
  int64_t lbj;
  std::vector<int64_t> moves;
  uint64_t buff;

  rfj = _right_forward_jumps(board);
  lfj = _left_forward_jumps(board);
  rbj = _right_backward_jumps(board);
  lbj = _left_backward_jumps(board);
  
  if ((rfj | lfj | rbj | lbj) != 0) {
    buff = static_cast<uint64_t>(rfj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffeff << i));
    }

    buff = static_cast<uint64_t>(lfj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffbff << i));
    }

    buff = static_cast<uint64_t>(rbj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffeff << (i - 8)));
    }

    buff = static_cast<uint64_t>(lbj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffbff << (i - 10)));
    }
  }
  return (moves);
}

std::vector<int64_t> _jumps_from(GameBoard board, int64_t piece) {
  /*
      Returns list of all possible jumps from the piece indicated.

      The argument piece should be of the form 2**n, where n + 1 is
      the square of the piece in question (using the internal numeric
      representaiton of the board).
  */
  int64_t rfj;
  int64_t lfj;
  int64_t rbj;
  int64_t lbj;
  std::vector<int64_t> moves;
  uint64_t buff;

  if (board.active == BLACK_PLAYER) {
    rfj = ((board.empty >> 8) & (board.pieces[board.passive] >> 4) & piece);
    lfj = ((board.empty >> 10) & (board.pieces[board.passive] >> 5) & piece);
    if (piece & board.backward[board.active]) { // piece at square is a king
      rbj = ((board.empty << 8) & (board.pieces[board.passive] << 4) & piece);
      lbj = ((board.empty << 10) & (board.pieces[board.passive] << 5) & piece);
    } else {
      rbj = 0;
      lbj = 0;
    }
  } else {
    rbj = ((board.empty << 8) & (board.pieces[board.passive] << 4) & piece);
    lbj = ((board.empty << 10) & (board.pieces[board.passive] << 5) & piece);
    if (piece & board.forward[board.active]) { // piece at square is a king
      rfj = ((board.empty >> 8) & (board.pieces[board.passive] >> 4) & piece);
      lfj = ((board.empty >> 10) & (board.pieces[board.passive] >> 5) & piece);
    } else {
      rfj = 0;
      lfj = 0;
    }
  }
  
  if ((rfj | lfj | rbj | lbj) != 0) {
    buff = static_cast<uint64_t>(rfj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffeff << i));
    }

    buff = static_cast<uint64_t>(lfj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffbff << i));
    }

    buff = static_cast<uint64_t>(rbj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffeff << (i - 8)));
    }

    buff = static_cast<uint64_t>(lbj);
    for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
      if ((buff & 1) == 1)
        moves.push_back((0xfffffffffffffbff << (i - 10)));
    }
  }
  return moves;
}
