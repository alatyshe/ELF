#include "CheckersBoard.h"

#define myassert(p, text) \
  do {                    \
    if (!(p)) {           \
      printf((text));     \
    }                     \
  } while (0)

void          ClearBoard(CheckersBoard* board) {
 // Инициализация состояния игры
  board->current_player = BLACK_PLAYER;
  board->game_ended = false;
  board->_ply = 1;
  board->_last_move = M_INVALID;

  board->next_bit_y = -1;
  board->next_bit_x = -1;

  // Заполнение начальной позиции шашек
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if ((y + x) % 2 == 0)  // Клетки с четной суммой координат - белые
        board->board[y][x] = EMPTY;
      else if (y < 3)        // Три верхних ряда черных клеток - черные шашки
        board->board[y][x] = WHITE_PAWN;
      else if (y > 4)        // Три нижних ряда черных клеток - белые шашки
        board->board[y][x] = BLACK_PAWN;
      else                   // Остальные черные клетки - пустые
        board->board[y][x] = EMPTY;
    }
  }
}
  

void        CheckersPlay(CheckersBoard *board, Coord action_index) {
  std::vector<std::array<int, 2>> tmp_moves;
  auto action = moves::i_to_m.find(action_index);
  int y_start = action->second[0] / 8;
  int x_start = action->second[0] % 8;
  int y_dest = action->second[1] / 8;
  int x_dest = action->second[1] % 8;

  int buff = board->board[y_start][x_start];
  int y = y_start;
  int x = x_start;
  int dir_y = y_start - y_dest > 0 ? DOWN : UP;
  int dir_x = x_start - x_dest < 0 ? RIGHT : LEFT;

  while (y != y_dest && x != x_dest) {
    y += dir_y;
    x += dir_x;
    if (board->board[y][x] != 0) {
      board->board[y][x] = 0;

      if (buff > 1 || buff < -1){
        tmp_moves = _kingJumps(*board, y_dest, x_dest);
      } else {
        tmp_moves = _pawnJumps(*board, y_dest, x_dest);
      }
      if (tmp_moves.size() > 0) {
        board->next_bit_y = y_dest;
        board->next_bit_x = x_dest;
      } else {
        board->next_bit_y = -1;
        board->next_bit_x = -1;
      }   
    }
  }
  board->board[y_dest][x_dest] = buff;
  board->board[y_start][x_start] = 0;
  
  if (board->next_bit_y == -1)
    if (board->current_player == WHITE_PLAYER)
      board->current_player = BLACK_PLAYER;
    else
      board->current_player = WHITE_PLAYER;

  _tryConvertIntoKing(board, y_dest, x_dest);
  board->_ply++;
  board->_last_move = action_index;
}


std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(CheckersBoard board) {
  std::vector<std::array<int, 2>> valid_moves;
  std::array<int, TOTAL_NUM_ACTIONS> result;
  std::string move_buff;
  int total_moves = 0;

  valid_moves = getAllMoves(board);
  result.fill(0);

  for (int i = 0; i < valid_moves.size(); i++) {
    move_buff = std::to_string(valid_moves[i][0]) + " => "  + std::to_string(valid_moves[i][1]);
    // print moves
    result[moves::m_to_i.find(move_buff)->second] = 1;
    total_moves += 1;
  }
  return result;
}


bool CheckersTryPlay(CheckersBoard board, Coord c) {
  std::array<int, TOTAL_NUM_ACTIONS> res = GetValidMovesBinary(board);

  if (res[c])
    return true;
  return false;
}


bool          CheckersIsOver(CheckersBoard board) {
  return getAllMoves(board).size() == 0;
}


// translates the board in 8x8 format 
//      3: our kings 
//      1: our pawns 
//      -3: enemy kings 
//      -1: enemy pawns
std::array<std::array<int, 8>, 8> GetObservation(const CheckersBoard board, int player) {
  std::array<std::array<int, 8>, 8> res;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (player == BLACK_PLAYER) {
        res[y][x] = board.board[y][x];
      } else {
        res[y][x] = board.board[7 - y][7 - x] * -1;
      }
    }
  }
  return res;
}


std::array<std::array<int, 8>, 8> GetTrueObservation(const CheckersBoard board) {
  std::array<std::array<int, 8>, 8> res;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
        res[y][x] = board.board[y][x];
    }
  }
  return res;
}


std::string   GetTrueObservationStr(const CheckersBoard board) {
  std::stringstream ss;

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (board.board[y][x] == WHITE_KING)
        ss << RED_C << " (" << std::setw(2) << std::right << y * 8 + x << ")K" << COLOR_END;
      else if (board.board[y][x] == WHITE_PAWN)
        ss << RED_C << " (" << std::setw(2) << std::right << y * 8 + x << ")M" << COLOR_END;
      else if (board.board[y][x] == BLACK_KING)
        ss << GREEN_C << " (" << std::setw(2) << std::right << y * 8 + x << ")K" << COLOR_END;
      else if (board.board[y][x] == BLACK_PAWN)
        ss << GREEN_C << " (" << std::setw(2) << std::right << y * 8 + x << ")M" << COLOR_END;
      else
        ss << " (" << std::setw(2) << std::right << y * 8 + x << ")E";
    }
    ss << std::endl;
  }
  return ss.str();
}


void CheckersCopyBoard(CheckersBoard* dst, const CheckersBoard* src) {
  myassert(dst, "dst cannot be nullptr");
  myassert(src, "src cannot be nullptr");

  memcpy(dst, src, sizeof(CheckersBoard));
}



std::vector<std::array<int, 2>> getJumps(CheckersBoard board) {
  std::vector<std::array<int, 2>> result;
  std::vector<std::array<int, 2>> tmp;

  int pawn;
  int king;
  if (board.current_player == WHITE_PLAYER) {
    pawn = WHITE_PAWN;
    king = WHITE_KING;
  } else {
    pawn = BLACK_PAWN;
    king = BLACK_KING;
  }

  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (board.board[y][x] == pawn || board.board[y][x] == king) {
        if (board.board[y][x] == pawn) {
          tmp = _pawnJumps(board, y, x);
        } else {
          tmp = _kingJumps(board, y, x);
        }

        if (tmp.size() != 0) {
          result.insert(
            result.end(),
            std::make_move_iterator(tmp.begin()),
            std::make_move_iterator(tmp.end()));
        }
      }
    }
  }
  return(result);
}


std::vector<std::array<int, 2>> getMoves(CheckersBoard board) {
  std::vector<std::array<int, 2>> result;
  std::vector<std::array<int, 2>> tmp;

  int pawn;
  int king;
  if (board.current_player == WHITE_PLAYER) {
    pawn = WHITE_PAWN;
    king = WHITE_KING;
  } else {
    pawn = BLACK_PAWN;
    king = BLACK_KING;
  }
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      if (board.board[y][x] == pawn || board.board[y][x] == king) {
        if (board.board[y][x] == pawn)
          tmp = _pawnMoves(board, y, x);
        else
          tmp = _kingMoves(board, y, x);

        if (tmp.size() != 0) {
          result.insert(
            result.end(),
            std::make_move_iterator(tmp.begin()),
            std::make_move_iterator(tmp.end()));
        }
      }
    }
  }
  return result;
}









std::vector<std::array<int, 2>> getAllMoves(CheckersBoard board) {  
  std::vector<std::array<int, 2>> result;

  if (board.next_bit_y != -1){
    if (board.board[board.next_bit_y][board.next_bit_x] > 1
        || board.board[board.next_bit_y][board.next_bit_x] < -1)
      return _kingJumps(board, board.next_bit_y, board.next_bit_x);
    else
      return _pawnJumps(board, board.next_bit_y, board.next_bit_x);
  }
  result = getJumps(board);
  if (result.size() > 0)
    return result;
  return getMoves(board);
}

void _tryConvertIntoKing(CheckersBoard *board, int y, int x) {
  if (board->board[y][x] == WHITE_PAWN && y == 7)
    board->board[y][x] = WHITE_KING;
  if (board->board[y][x] == BLACK_PAWN && y == 0)
    board->board[y][x] = BLACK_KING;
}


bool _coordOverflow(int num) {
  return (num < 0 || num > 7);
}






























// нормально работает
std::vector<std::array<int, 2>> _pawnMoves(CheckersBoard board, int y, int x) {
  std::vector<std::array<int, 2>> moves;
  int start, dest;

  start = y * 8 + x;
  if (board.current_player == WHITE_PLAYER) {
    if (!_coordOverflow(y + UP) && !_coordOverflow(x + LEFT)
          && board.board[y + UP][x + LEFT] == 0) {
      dest = (y + UP) * 8 + x + LEFT;
      moves.push_back(std::array<int, 2>{start, dest});
    }
    if (!_coordOverflow(y + UP) && !_coordOverflow(x + RIGHT)
          && board.board[y + UP][x + RIGHT] == 0) {
      dest = (y + UP) * 8 + x + RIGHT;
      moves.push_back(std::array<int, 2>{start, dest});
    }
  } else if (board.current_player == BLACK_PLAYER) {
    if (!_coordOverflow(y + DOWN) && !_coordOverflow(x + LEFT)
          && board.board[y + DOWN][x + LEFT] == 0) {
      dest = (y + DOWN) * 8 + x + LEFT;
      moves.push_back(std::array<int, 2>{start, dest});
    }
    if (!_coordOverflow(y + DOWN) && !_coordOverflow(x + RIGHT)
          && board.board[y + DOWN][x + RIGHT] == 0) {
      dest = (y + DOWN) * 8 + x + RIGHT;
      moves.push_back(std::array<int, 2>{start, dest});
    }
  }
  return moves;
}
// нормально работает
std::vector<std::array<int, 2>> _kingMoves(CheckersBoard board, int y, int x) {
  std::vector<std::array<int, 2>> moves;
  int tmp_y, tmp_x;
  int start, dest;
  
  start = y * 8 + x;

  tmp_y = y + UP;
  tmp_x = x + LEFT;
  while (!_coordOverflow(tmp_y) && !_coordOverflow(tmp_x) 
        && board.board[tmp_y][tmp_x] == 0) {

    dest = tmp_y * 8 + tmp_x;
    moves.push_back(std::array<int, 2>{start, dest});
    
    tmp_y += UP;
    tmp_x += LEFT;
  }

  tmp_y = y + UP;
  tmp_x = x + RIGHT;
  while (!_coordOverflow(tmp_y) && !_coordOverflow(tmp_x) 
        && board.board[tmp_y][tmp_x] == 0) {

    dest = tmp_y * 8 + tmp_x;
    moves.push_back(std::array<int, 2>{start, dest});

    tmp_y += UP;
    tmp_x += RIGHT;
  }

  tmp_y = y + DOWN;
  tmp_x = x + LEFT;
  while (!_coordOverflow(tmp_y) && !_coordOverflow(tmp_x) 
        && board.board[tmp_y][tmp_x] == 0) {

    dest = tmp_y * 8 + tmp_x;
    moves.push_back(std::array<int, 2>{start, dest});

    tmp_y += DOWN;
    tmp_x += LEFT;
  }

  tmp_y = y + DOWN;
  tmp_x = x + RIGHT;
  while (!_coordOverflow(tmp_y) && !_coordOverflow(tmp_x) 
        && board.board[tmp_y][tmp_x] == 0) {

    dest = tmp_y * 8 + tmp_x;
    moves.push_back(std::array<int, 2>{start, dest});

    tmp_y += DOWN;
    tmp_x += RIGHT;
  }

  return moves;
}






// Проверяет возможно прыгнуть дамки из текущего
bool  _kingJumpCheck(CheckersBoard board, int y, int x, int dir_y, int dir_x) {
  int dest_y, dest_x;
  int enemy_y, enemy_x;
  int enemy_king, enemy_pawn;

  if (board.current_player == WHITE_PLAYER) {
    enemy_king = BLACK_KING;
    enemy_pawn = BLACK_PAWN;
  } else {
    enemy_king = WHITE_KING;
    enemy_pawn = WHITE_PAWN;
  }

  // Для дамок следует пропустить все пустые клетки в данном направлении
  dest_y = y + dir_y;
  dest_x = x + dir_x;

  while (true) {
    if (_coordOverflow(dest_y) || _coordOverflow(dest_x))
      return false;
    if (board.board[dest_y][dest_x] != 0)
      break ;
    dest_y += dir_y;
    dest_x += dir_x;
  }
  // Координаты шашки, которая будет бита
  // для простой шашки это следующая клетка
  // для дамки - первая непустая в данном направлении
  enemy_y = dest_y;
  enemy_x = dest_x;
  if (board.board[enemy_y][enemy_x] != enemy_king 
      && board.board[enemy_y][enemy_x] != enemy_pawn)
    return false;
  
  dest_y = enemy_y + dir_y;
  dest_x = enemy_x + dir_x;
  if (!_coordOverflow(dest_y) && !_coordOverflow(dest_x) 
      && board.board[dest_y][dest_x] == 0) {
    return true;
  }
  return false;
}


// Прыжок дамки в конкретном направлении
std::vector<std::array<int, 2>> _kingJumpInDirection(CheckersBoard board, int y, int x, int dir_y, int dir_x) {
  std::vector<std::array<int, 2>> jumps;
  int start, dest;
  // int enemy_buff;
  int dest_y, dest_x;
  int enemy_y, enemy_x;
  int enemy_king, enemy_pawn;

  start = y * 8 + x;
  if (board.current_player == WHITE_PLAYER) {
    enemy_king = BLACK_KING;
    enemy_pawn = BLACK_PAWN;
  } else {
    enemy_king = WHITE_KING;
    enemy_pawn = WHITE_PAWN;
  }

  dest_y = y;
  dest_x = x;
  // Скипаем все свободные клетки до первой попавшейся
  while (true) {
    dest_y += dir_y;
    dest_x += dir_x;

    if (_coordOverflow(dest_y) || _coordOverflow(dest_x))
      return jumps;
    if (board.board[dest_y][dest_x] != 0)
      break ;
  }

  // Координаты шашки, которая будет бита
  enemy_y = dest_y;
  enemy_x = dest_x;

  if (_coordOverflow(enemy_y) || _coordOverflow(enemy_x)
        || (board.board[enemy_y][enemy_x] != enemy_king 
         && board.board[enemy_y][enemy_x] != enemy_pawn))
    return jumps;

  board.board[enemy_y][enemy_x] = 0;
  dest_y = enemy_y + dir_y;
  dest_x = enemy_x + dir_x;
  if (_coordOverflow(dest_y) && _coordOverflow(dest_x) 
      && board.board[dest_y][dest_x] != 0)
    return jumps;

  // enemy_buff = board.board[enemy_y][enemy_x];
  // board.board[enemy_y][enemy_x] = 0;

  while (true) {    
    if (_coordOverflow(dest_y) 
        || _coordOverflow(dest_x) 
        || board.board[dest_y][dest_x] != 0)
      break ;
    // проверяем будет ли возможность побить после текущего хода
    if (   _kingJumpCheck(board, dest_y, dest_x, UP, LEFT) 
        || _kingJumpCheck(board, dest_y, dest_x, UP, RIGHT)
        || _kingJumpCheck(board, dest_y, dest_x, DOWN, LEFT)
        || _kingJumpCheck(board, dest_y, dest_x, DOWN, RIGHT)){

      jumps.clear();
      dest = dest_y * 8 + dest_x;
      jumps.push_back(std::array<int, 2>{start, dest});
      break;
    }
    dest = dest_y * 8 + dest_x;
    jumps.push_back(std::array<int, 2>{start, dest});

    dest_y += dir_y;
    dest_x += dir_x;
  }
  // board.board[enemy_y][enemy_x] = enemy_buff;
  return jumps;
}


// Проходим по всем направлениям для конкретной дамки
std::vector<std::array<int, 2>> _kingJumps(CheckersBoard board, int y, int x) {
  std::vector<std::array<int, 2>> moves;
  std::vector<std::array<int, 2>> buff;

  buff = _kingJumpInDirection(board, y, x, UP, LEFT);
  moves.insert(
    moves.end(),
    std::make_move_iterator(buff.begin()),
    std::make_move_iterator(buff.end()));
  
  buff = _kingJumpInDirection(board, y, x, UP, RIGHT);
  moves.insert(
    moves.end(),
    std::make_move_iterator(buff.begin()),
    std::make_move_iterator(buff.end()));

  buff = _kingJumpInDirection(board, y, x, DOWN, LEFT);
  moves.insert(
    moves.end(),
    std::make_move_iterator(buff.begin()),
    std::make_move_iterator(buff.end()));
  
  buff = _kingJumpInDirection(board, y, x, DOWN, RIGHT);
  moves.insert(
    moves.end(),
    std::make_move_iterator(buff.begin()),
    std::make_move_iterator(buff.end()));

  return moves;
}





std::vector<std::array<int, 2>> _pawnJumps(CheckersBoard board, int y, int x) {
  std::vector<std::array<int, 2>> moves;
  std::array<int, 2> buff;

  buff = _pawnJumpInDirection(board, y, x, UP, LEFT);
  if (buff[0] != -1)
    moves.push_back(buff);
  
  buff = _pawnJumpInDirection(board, y, x, UP, RIGHT);
  if (buff[0] != -1)
    moves.push_back(buff);

  buff = _pawnJumpInDirection(board, y, x, DOWN, LEFT);
  if (buff[0] != -1)
    moves.push_back(buff);
  
  buff = _pawnJumpInDirection(board, y, x, DOWN, RIGHT);
  if (buff[0] != -1)
    moves.push_back(buff);

  return moves;
}


std::array<int, 2> _pawnJumpInDirection(CheckersBoard board, int y, int x, int dir_y, int dir_x) {
  int enemy_king, enemy_pawn;
  int dest_y, dest_x;
  int enemy_y, enemy_x;

  std::array<int, 2> jump;
  jump[0] = -1;
  jump[1] = -1;

  if (board.current_player == WHITE_PLAYER) {
    enemy_king = BLACK_KING;
    enemy_pawn = BLACK_PAWN;
  } else {
    enemy_king = WHITE_KING;
    enemy_pawn = WHITE_PAWN;
  }

  // Координаты шашки, которая будет бита
  // для простой шашки это следующая клетка
  enemy_y = y + dir_y;
  enemy_x = x + dir_x;

  if (_coordOverflow(enemy_y) || _coordOverflow(enemy_x)
         || (board.board[enemy_y][enemy_x] != enemy_king 
          && board.board[enemy_y][enemy_x] != enemy_pawn))
    return jump;
  
  dest_y = enemy_y + dir_y;
  dest_x = enemy_x + dir_x;
  if (!_coordOverflow(dest_y) && !_coordOverflow(dest_x) 
      && board.board[dest_y][dest_x] == 0){
    jump[0] = y * 8 + x;
    jump[1] = dest_y * 8 + dest_x;  
  }
  return jump;
}

