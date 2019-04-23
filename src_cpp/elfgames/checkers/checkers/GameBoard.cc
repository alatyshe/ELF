#include "GameBoard.h"

#define myassert(p, text) \
	do {                    \
		if (!(p)) {           \
			printf((text));     \
		}                     \
	} while (0)


// Инициализация состояния игры
void          ClearBoard(GameBoard* board) {
	board->current_player = BLACK_PLAYER;
	board->game_ended = false;
	board->_ply = 1;

	board->jump_y = -1;
	board->jump_x = -1;

	board->white_must_leave_base = false;
	board->black_must_leave_base = false;
	board->_last_move = M_INVALID;

	// Заполнение начальной позиции шашек
	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			// if (y < 4 && x < 4)
			//   board->board[y][x] = WHITE_PLAYER;
			// else if (y > 3 && x > 3)        // Три нижних ряда черных клеток - белые шашки
			//   board->board[y][x] = BLACK_PLAYER;
			// else                   // Остальные черные клетки - пустые
				board->board[y][x] = EMPTY;
		}
	}
	std::array<std::array<int, 8>, 8> init_board = {{
		{-1, -1, -1, -1, 0, 0, 0,	0},
		{-1, -1, -1, -1, 0, 0, 0,	0},
		{-1, -1, -1, -1, 1, 0, 1, 1},
		{0, 0, 0, -1, 0, 0, 0, 0},
		{0, 0, -1, 1, 0, 0, 0, 0},
		{0, 0, 0, 0, 1, 1, 1, 1},
		{-1, -1, 0, 0, 1, 1, 1, 1},
		{0, 0, 0, 0, 1, 1, 1, 1}
	}};

	board->board = init_board;
}
	

// делает шаг плучая индекс action
void        CheckersPlay(GameBoard *board, int64_t action_index) {
	bool jump = false;
	board->white_must_leave_base = false;
	board->black_must_leave_base = false;
	
	if (action_index != 416) {
		auto action = moves::i_to_m.find(action_index);
		int y_start = action->second[0] / 8;
		int x_start = action->second[0] % 8;
		int y_dest = action->second[1] / 8;
		int x_dest = action->second[1] % 8;
		
		if (std::abs(y_start - y_dest) > 1 || std::abs(x_start - x_dest) > 1) {
			if (_getJump(*board, y_dest, x_dest).size() > 1) {
				board->jump_y = y_dest;
				board->jump_x = x_dest;
				jump = true;
			}
		}

		board->board[y_dest][x_dest] = board->board[y_start][x_start];
		board->board[y_start][x_start] = TEMP;
	}
	int white_pawns_on_base = 0;
	int black_pawns_on_base = 0;
	if (!jump) {

		for (int y = 0; y < 8; y++) {
			for (int x = 0; x < 8; x++) {
				if (board->board[y][x] == TEMP)
					board->board[y][x] = EMPTY;
				if (y < 4 && x < 4 && board->board[y][x] == WHITE_PLAYER)
					white_pawns_on_base++;
				else if (y > 3 && x > 3 && board->board[y][x] == BLACK_PLAYER)
					black_pawns_on_base++;
			}
		}
		if (black_pawns_on_base == 0 && white_pawns_on_base > 0)
			board->white_must_leave_base = true;
		if (white_pawns_on_base == 0 && black_pawns_on_base > 0)
			board->black_must_leave_base = true;

		board->jump_y = -1;
		board->jump_x = -1;
		if (board->current_player == WHITE_PLAYER)
			board->current_player = BLACK_PLAYER;
		else
			board->current_player = WHITE_PLAYER;
	}

	board->_ply++;
	board->_last_move = action_index;
}


// Принимает доску и по ней рассчитывает валидные шаги возвращая вектор 
// из всех возможных шагов ставя на валидные - 1 
std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(GameBoard board) {
	std::vector<std::array<int, 2>> valid_moves;
	std::array<int, TOTAL_NUM_ACTIONS> result;
	std::string move_buff;

	valid_moves = _getAllMoves(board);
	result.fill(0);

	for (int i = 0; i < valid_moves.size(); i++) {
		move_buff = std::to_string(valid_moves[i][0]) + " => "  + std::to_string(valid_moves[i][1]);
		// std::cout << move_buff << std::endl;
		result[moves::m_to_i.find(move_buff)->second] = 1;
	}
	return result;
}


bool CheckersTryPlay(GameBoard board, int64_t c) {
	std::array<int, TOTAL_NUM_ACTIONS> res = GetValidMovesBinary(board);

	if (res[c])
		return true;
	return false;
}


// Проверка - окончена ли игра
bool          CheckersIsOver(GameBoard board) {
	int   black_pawns_on_base = 0;
	int   white_pawns_on_base = 0;

	for (int y = 0; y < 8; y++){
		for (int x = 0; x < 8; x++) {
			if (y < 4 && x < 4 && board.board[y][x] == BLACK_PLAYER)
				black_pawns_on_base++;
			else if (y > 3 && x > 3 && board.board[y][x] == WHITE_PLAYER)
				white_pawns_on_base++;
		}
	}
	if (black_pawns_on_base == 16 || white_pawns_on_base == 16)
		return true;
	
	return false;
}


// translates the board in 8x8 format 
//      1: our pawns  
//      -1: enemy pawns
std::array<std::array<int, 8>, 8> GetObservation(GameBoard board, int player) {
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


std::array<std::array<int, 8>, 8> GetTrueObservation(const GameBoard board) {
	std::array<std::array<int, 8>, 8> res;

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
				res[y][x] = board.board[y][x];
		}
	}
	return res;
}


std::string   GetTrueObservationStr(const GameBoard board) {
	std::stringstream ss;


	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (board.board[y][x] == WHITE_PLAYER)
				ss << RED_B << " (" << std::setw(2) << std::right << y * 8 + x << ")W" << COLOR_END;
			else if (board.board[y][x] == BLACK_PLAYER)
				ss << GREEN_B << " (" << std::setw(2) << std::right << y * 8 + x << ")B" << COLOR_END;
			else
				ss << " (" << std::setw(2) << std::right << y * 8 + x << ")E";
		}
		ss << std::endl;
	}
	return ss.str();
}


void CheckersCopyBoard(GameBoard* dst, const GameBoard* src) {
	myassert(dst, "dst cannot be nullptr");
	myassert(src, "src cannot be nullptr");

	memcpy(dst, src, sizeof(GameBoard));
}



bool compareBoards(GameBoard b1, GameBoard b2) {
	int res = 0;
	
	for (int y = 0; y < BOARD_SIZE; y++) {
		for (int x = 0; x < BOARD_SIZE; x++)
			res += (b1.board[y][x] != b1.board[y][x]);
	}
	res += (b1.current_player != b2.current_player);
	res += (b1.game_ended != b2.game_ended);
	res += (b1.jump_y != b2.jump_y);
	res += (b1.jump_x != b2.jump_x);
	res += (b1.white_must_leave_base != b2.white_must_leave_base);
	res += (b1.black_must_leave_base != b2.black_must_leave_base);
	res += (b1._last_move != b2._last_move);
	res += (b1._ply != b2._ply);

	return res == 0;
}






























std::vector<std::array<int, 2>> _getJump(GameBoard board, int y, int  x) {
	std::vector<std::array<int, 2>> jumps;
	int start, dest;

	start = y * 8 + x;
	jumps.push_back(std::array<int, 2>{-1, -1});

	if (x + 2 < 8 && board.board[y][x+1] != EMPTY && board.board[y][x+2] == EMPTY) {
		dest = y * 8 + x + 2;
		jumps.push_back(std::array<int, 2>{start, dest});
	}
	if (x - 2 >= 0 && board.board[y][x-1] != EMPTY && board.board[y][x-2] == EMPTY) {
		dest = y * 8 + x - 2;
		jumps.push_back(std::array<int, 2>{start, dest});
	}
	if (y + 2 < 8 && board.board[y+1][x] != EMPTY && board.board[y+2][x] == EMPTY) {
		dest = (y + 2) * 8 + x;
		jumps.push_back(std::array<int, 2>{start, dest});
	}
	if (y - 2 >= 0 && board.board[y-1][x] != EMPTY && board.board[y-2][x] == EMPTY) {
		dest = (y - 2) * 8 + x;
		jumps.push_back(std::array<int, 2>{start, dest}); 
	}
	return jumps;
}

std::vector<std::array<int, 2>> _getAllMovesFromBase(GameBoard board) {
	std::vector<std::array<int, 2>> moves;
	int y_start, x_start, y_dest, x_dest;

	int start, dest;
	if (board.current_player == WHITE_PLAYER){
		y_start = 0;
		y_dest = 4;
		x_dest = 4;
	} else {
		y_start = 4;
		y_dest = 8;
		x_dest = 8;
	}

	for (; y_start < y_dest; y_start++) {
		if (board.current_player == WHITE_PLAYER)
			x_start = 0;
		else
			x_start = 4;

		for (; x_start < x_dest; x_start++) {
			if (board.board[y_start][x_start] == board.current_player) {
				start = y_start * 8 + x_start;
				// jumps
				if (x_start + 2 < 8 && board.board[y_start][x_start+1] != EMPTY 
						&& board.board[y_start][x_start+2] == EMPTY) {
					dest = y_start * 8 + x_start + 2;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (x_start - 2 >= 0 && board.board[y_start][x_start-1] != EMPTY 
						&& board.board[y_start][x_start-2] == EMPTY) {
					dest = y_start * 8 + x_start - 2;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (y_start + 2 < 8 && board.board[y_start+1][x_start] != EMPTY 
						&& board.board[y_start+2][x_start] == EMPTY) {
					dest = (y_start + 2) * 8 + x_start;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (y_start - 2 >= 0 && board.board[y_start-1][x_start] != EMPTY 
						&& board.board[y_start-2][x_start] == EMPTY) {
					dest = (y_start - 2) * 8 + x_start;
					moves.push_back(std::array<int, 2>{start, dest}); 
				}
				// moves;
				if (x_start + 1 < 8 && board.board[y_start][x_start+1] == EMPTY) {
					dest = y_start * 8 + x_start + 1;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (x_start - 1 >= 0 && board.board[y_start][x_start-1] == EMPTY) {
					dest = y_start * 8 + x_start - 1;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (y_start + 1 < 8 && board.board[y_start+1][x_start] == EMPTY) {
					dest = (y_start + 1) * 8 + x_start;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (y_start - 1 >= 0 && board.board[y_start-1][x_start] == EMPTY) {
					dest = (y_start - 1) * 8 + x_start;
					moves.push_back(std::array<int, 2>{start, dest}); 
				}
			}
		}
	}
	return moves;
}


std::vector<std::array<int, 2>> _getJumps(GameBoard board) {
	std::vector<std::array<int, 2>> jumps;
	int start, dest;

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (board.board[y][x] == board.current_player) {
				start = y * 8 + x;
				
				if (x + 2 < 8 && board.board[y][x+1] != EMPTY && board.board[y][x+2] == EMPTY) {
					dest = y * 8 + x + 2;
					jumps.push_back(std::array<int, 2>{start, dest});
				}
				if (x - 2 >= 0 && board.board[y][x-1] != EMPTY && board.board[y][x-2] == EMPTY) {
					dest = y * 8 + x - 2;
					jumps.push_back(std::array<int, 2>{start, dest});
				}
				if (y + 2 < 8 && board.board[y+1][x] != EMPTY && board.board[y+2][x] == EMPTY) {
					dest = (y + 2) * 8 + x;
					jumps.push_back(std::array<int, 2>{start, dest});
				}
				if (y - 2 >= 0 && board.board[y-1][x] != EMPTY && board.board[y-2][x] == EMPTY) {
					dest = (y - 2) * 8 + x;
					jumps.push_back(std::array<int, 2>{start, dest}); 
				}
			}
		}
	}
	return jumps;
}


std::vector<std::array<int, 2>> _getMoves(GameBoard board) {
	std::vector<std::array<int, 2>> moves;
	int start, dest;

	for (int y = 0; y < 8; y++) {
		for (int x = 0; x < 8; x++) {
			if (board.board[y][x] == board.current_player) {
				start = y * 8 + x;
				
				if (x + 1 < 8 && board.board[y][x+1] == EMPTY) {
					dest = y * 8 + x + 1;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (x - 1 >= 0 && board.board[y][x-1] == EMPTY) {
					dest = y * 8 + x - 1;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (y + 1 < 8 && board.board[y+1][x] == EMPTY) {
					dest = (y + 1) * 8 + x;
					moves.push_back(std::array<int, 2>{start, dest});
				}
				if (y - 1 >= 0 && board.board[y-1][x] == EMPTY) {
					dest = (y - 1) * 8 + x;
					moves.push_back(std::array<int, 2>{start, dest}); 
				}
			}
		}
	}
	return moves;
}


std::vector<std::array<int, 2>> _getAllMoves(GameBoard board) {  
	std::vector<std::array<int, 2>> moves;
	std::vector<std::array<int, 2>> buff;

	if (board.jump_x != -1)
		return _getJump(board, board.jump_y, board.jump_x);

	if ((board.current_player == BLACK_PLAYER && board.black_must_leave_base) 
			|| (board.current_player == WHITE_PLAYER && board.white_must_leave_base))
		return _getAllMovesFromBase(board);

	moves = _getJumps(board);
	buff = _getMoves(board);
	moves.insert(
		moves.end(),
		std::make_move_iterator(buff.begin()),
		std::make_move_iterator(buff.end()));

	return moves;
}



