#include "CheckersBoard.h"

#define myassert(p, text) \
  do {                    \
    if (!(p)) {           \
      printf((text));     \
    }                     \
  } while (0)

void				ClearBoard(CheckersBoard* board) {
	// board->_total_moves = 0;
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

	board->_last_move = 0;

	board->empty = UNUSED_BITS ^ MASK ^ (board->pieces[BLACK_PLAYER] | board->pieces[WHITE_PLAYER]);

	board->jump = 0;
}

bool				CheckersPlay(CheckersBoard *board, int64_t action_index) {
	/*
		Updates the game state to reflect the effects of the input
		move.

		A legal move is represented by an integer with exactly two
		bits turned on: the old position and the new position.
	*/
	int64_t 				move;
	int64_t 				active;
	int64_t 				passive;
	int64_t 				taken_piece;
	int64_t 				destination;
	int 					buffer;
	uint64_t 				buff;

	auto index = moves::i_to_m.find(action_index);
	move = index->second[0];

	board->_last_move = action_index;
	active = board->active;
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

	// std::cout << get_true_state_str(*board) << std::endl;
	return false;
}




std::array<int, ALL_ACTIONS>		GetValidMovesBinary(CheckersBoard board, int player) {
	// std::vector<std::array<int64_t, 2>>	result;
	int 								buffer;
	std::vector<int64_t>				moves;
	std::string							move_buff;
	std::array<int, ALL_ACTIONS>		result;

	result.fill(0);
	if (player != board.active) {
		buffer = board.active;
		board.active = board.passive;
		board.passive = buffer;

		moves = _get_moves(board);
		for (auto i = moves.begin(); i != moves.end(); ++i) {
			move_buff = std::to_string(*i) + ", "  + std::to_string(_get_move_direction(board, *i, board.active));

			// // print moves			
			// std::cout << move_buff << " : |" << moves::m_to_i.find(move_buff)->second << "|" << std::endl;

			result[moves::m_to_i.find(move_buff)->second] = 1;
		}

		buffer = board.active;
		board.active = board.passive;
		board.passive = buffer;
	}
	else {
		moves = _get_moves(board);

		for (auto i = moves.begin(); i != moves.end(); ++i) {
			move_buff = std::to_string(*i) + ", "  + std::to_string(_get_move_direction(board, *i, board.active));
			
			// // print moves
			// std::cout << move_buff << " : |" << moves::m_to_i.find(move_buff)->second << "|" << std::endl;

			result[moves::m_to_i.find(move_buff)->second] = 1;
		}
	}

	return result;
}


std::vector<std::array<int64_t, 2>>	GetValidMovesNumberAndDirection(CheckersBoard board, int player) {
	std::vector<std::array<int64_t, 2>>	result;
	int 								buffer;
	std::vector<int64_t>				moves;
	std::array<int64_t, 2>				move_buff;

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
	}
	else {
		moves = _get_moves(board);
		for (auto i = moves.begin(); i != moves.end(); ++i) {
			move_buff = {*i, _get_move_direction(board, *i, board.active)};
			result.push_back(move_buff);
		}
	}
	return result;
}

bool				CheckersTryPlay(CheckersBoard board, Coord c) {
	std::array<int, ALL_ACTIONS> res = GetValidMovesBinary(board, board.active);
	if (res[c])
		return true;
	return false;
}

bool				is_over(CheckersBoard board) {
	return (_get_moves(board).size() == 0);
}

// // CheckersBoard*		clone() {
// // 	CheckersBoard *B = new CheckersBoard();

// //     B->backward = board->backward;
// //     B->forward = board->forward;
// //     B->pieces = board->pieces;
// //     B->empty = board->empty;

// // 	B->active = board->active;
// // 	B->passive = board->passive;

// //     B->jump = board->jump;
// //     B->mandatory_jumps = board->mandatory_jumps;

// //     return (B);
// // }

// std::array<std::array<int, 8>, 8>	get_state_matrix_enemy(CheckersBoard *board, int player) {
//     std::array<std::array<int, 8>, 8> observation = board->get_observation(player);

//     for (int y = 0; y < 8; y++) {
//         for (int x = 0; x < 8; x++) {
//             if (observation[y][x] > 0)
//                 observation[y][x] = 0;
//         }
//     }
//     return (observation);
// }

// std::array<std::array<int, 8>, 8>	get_state_matrix_own(CheckersBoard *board, int player) {
//     std::array<std::array<int, 8>, 8> observation = board->get_observation(player);

//     for (int y = 0; y < 8; y++) {
//         for (int x = 0; x < 8; x++) {
//             if (observation[y][x] < 0)
//                 observation[y][x] = 0;
//         }
//     }
//     return (observation);
// }

// std::string			get_state_str(const Board board, int player) {
// 	std::array<std::array<int, 8>, 8> observation = get_observation(board, player);
// 	std::string str = "";
// 	std::string id;
// 	std::string man;
// 	std::string king;

//     for (int y = 0; y < 8; y++) {
//         for (int x = 0; x < 8; x++) {
//             id = " (" + std::to_string(x) + "," + std::to_string(y) + ")E";
//             man = " (" + std::to_string(x) + "," + std::to_string(y) + ")M";
//             king = " (" + std::to_string(x) + "," + std::to_string(y) + ")K";
            
//             if (observation[y][x] == -1) {
//                 id = "\x1b[6;31;40m" + man + "\x1b[0m";
//             } else if (observation[y][x] == -3) {
//                 id = "\x1b[6;31;40m" + king + "\x1b[0m";
//             } else if (observation[y][x] == 1) {
//                 id = "\x1b[6;32;40m" + man + "\x1b[0m";
//             } else if (observation[y][x] == 3) {
//                 id = "\x1b[6;32;40m" + king + "\x1b[0m";
//             }
//             str = str + id;
//         }
//         str = str + "\n";
//     }
//     return(str);
// }


std::array<std::array<int, 8>, 8>	get_observation(CheckersBoard board, int player) {
	std::array<std::array<int, 8>, 8>	board_out;
	int		x;
	int		y;
	int		buff;
	int64_t bin_black_pawn;
	int64_t bin_black_king;
	int64_t bin_white_pawn;
	int64_t bin_white_king;

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

std::array<std::array<int, 8>, 8>	get_true_state(CheckersBoard board) {
	return (get_observation(board, BLACK_PLAYER));
}

std::string			get_true_state_str(const CheckersBoard board) {
	std::array<std::array<int, 8>, 8> observation = get_true_state(board);
	std::string str = "";
	std::string id;
	std::string man;
	std::string king;

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            id = " (" + std::to_string(x) + "," + std::to_string(y) + ")E";
            man = " (" + std::to_string(x) + "," + std::to_string(y) + ")M";
            king = " (" + std::to_string(x) + "," + std::to_string(y) + ")K";
            
            if (observation[y][x] == -1) {
                id = "\x1b[6;31;40m" + man + "\x1b[0m";
            } else if (observation[y][x] == -3) {
                id = "\x1b[6;31;40m" + king + "\x1b[0m";
            } else if (observation[y][x] == 1) {
                id = "\x1b[6;32;40m" + man + "\x1b[0m";
            } else if (observation[y][x] == 3) {
                id = "\x1b[6;32;40m" + king + "\x1b[0m";
            }
            str = str + id;
        }
        str = str + "\n";
    }
    return(str);
}

void				set_current_player(CheckersBoard *board, int player) {
    int 				buffer;

    if (player != board->active)
    {
    	buffer = board->active;
		board->active = board->passive;
		board->passive = buffer;
    }
}


void				CheckersCopyBoard(CheckersBoard* dst, const CheckersBoard* src) {
  myassert(dst, "dst cannot be nullptr");
  myassert(src, "src cannot be nullptr");

  memcpy(dst, src, sizeof(CheckersBoard));
}





































int64_t	    		_right_forward(CheckersBoard board) {
	return ((board.empty >> 4) & board.forward[board.active]);
}

int64_t	    		_left_forward(CheckersBoard board) {
	return ((board.empty >> 5) & board.forward[board.active]);
}

int64_t	    		_right_backward(CheckersBoard board) {
	return ((board.empty << 4) & board.backward[board.active]);
}

int64_t	    		_left_backward(CheckersBoard board) {
	return ((board.empty << 5) & board.backward[board.active]);
}

int64_t	    		_right_forward_jumps(CheckersBoard board) {
	return ((board.empty >> 8) & (board.pieces[board.passive] >> 4) & board.forward[board.active]);
}

int64_t	    		_left_forward_jumps(CheckersBoard board) {
	return ((board.empty >> 10) & (board.pieces[board.passive] >> 5) & board.forward[board.active]);
}

int64_t	    		_right_backward_jumps(CheckersBoard board) {
	return ((board.empty << 8) & (board.pieces[board.passive] << 4) & board.backward[board.active]);
}

int64_t	    		_left_backward_jumps(CheckersBoard board) {
	return ((board.empty << 10) & (board.pieces[board.passive] << 5) & board.backward[board.active]);
}


int64_t	    		_get_move_direction(CheckersBoard board, int64_t move, int player) {
	if (move < 0)
		move = -move;
	return (board.pieces[player] < (board.pieces[player] ^ move));
}

std::vector<int64_t>		_get_moves(CheckersBoard board) {
	/*
		Returns a list of all possible moves.

		A legal move is represented by an integer with exactly two
		bits turned on: the old position and the new position.

		Jumps are indicated with a negative sign.
	*/
	int64_t					rf;
	int64_t					lf;
	int64_t					rb;
	int64_t					lb;
	std::vector<int64_t>	jumps;
	std::vector<int64_t>	moves;

	uint64_t 				buff;
	// First check if we are in a jump sequence
	// if (board.jump) {
	// 	return (board.mandatory_jumps);
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

std::vector<int64_t>		_get_jumps(CheckersBoard board) {
	/*
		Returns a list of all possible jumps.

		A legal move is represented by an integer with exactly two
		bits turned on: the old position and the new position.

		Jumps are indicated with a negative sign.
	*/
	int64_t					rfj;
	int64_t					lfj;
	int64_t					rbj;
	int64_t					lbj;
	std::vector<int64_t>	moves;

	uint64_t 				buff;

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

std::vector<int64_t>		_jumps_from(CheckersBoard board, int64_t piece) {
    /*
        Returns list of all possible jumps from the piece indicated.

        The argument piece should be of the form 2**n, where n + 1 is
        the square of the piece in question (using the internal numeric
        representaiton of the board).
    */
	int64_t	rfj;
	int64_t	lfj;
	int64_t	rbj;
	int64_t	lbj;
	std::vector<int64_t>	moves;

	uint64_t 				buff;

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
    }
    else {
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






















































// OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT 
// OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT 
// OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT OTHER SHIT 



