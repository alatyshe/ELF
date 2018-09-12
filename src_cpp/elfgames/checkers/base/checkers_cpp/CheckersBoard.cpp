#include "CheckersBoard.hpp"

CheckersBoard::CheckersBoard() {
	this->board_n = 8;

	this->forward = {0, 0};
	this->backward = {0, 0};
	this->pieces = {0, 0};
	this->new_game();
}

CheckersBoard::~CheckersBoard() { ; }

void				CheckersBoard::new_game() {
	this->active = BLACK;
	this->passive = WHITE;

	this->empty = 0;

	this->forward[BLACK] = 0x1eff;
	this->backward[BLACK] = 0;
	this->pieces[BLACK] = (this->forward[BLACK]) | (this->backward[BLACK]);

	this->forward[WHITE] = 0;
	this->backward[WHITE] = 0x7fbc00000;
	this->pieces[WHITE] = (this->forward[WHITE]) | (this->backward[WHITE]);

	this->empty = UNUSED_BITS ^ MASK ^ (this->pieces[BLACK] | this->pieces[WHITE]);

	this->jump = 0;
}

bool				CheckersBoard::make_move(std::array<int64_t, 2> action) {
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

	move = action[0];

	active = this->active;
	passive = this->passive;
	buffer = 0;
	if (move < 0) {
		move *= -1;

		buff = static_cast<uint64_t>(move);
		for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
			if ((buff & 1) == 1)
				buffer += i;
		}
		taken_piece = 1 << buffer / 2; 
		this->pieces[passive] ^= taken_piece;
		if (this->forward[passive] & taken_piece)
			this->forward[passive] ^= taken_piece;
		if (this->backward[passive] & taken_piece)
			this->backward[passive] ^= taken_piece;
		this->jump = 1;
	}

	this->pieces[active] ^= move;
	if (this->forward[active] & move)
		this->forward[active] ^= move;
	if (this->backward[active] & move)
		this->backward[active] ^= move;

	destination = move & this->pieces[active];
	this->empty = UNUSED_BITS ^ MASK ^ (this->pieces[BLACK] | this->pieces[WHITE]);

	if (this->jump) {
		this->mandatory_jumps = this->_jumps_from(destination);
		if (this->mandatory_jumps.size() != 0)
			return true;
	}

	if (active == BLACK && (destination & 0x780000000) != 0)
		this->backward[BLACK] |= destination;
	else if (active == WHITE && (destination & 0xf) != 0)
		this->forward[WHITE] |= destination;

	this->jump = 0;
	buffer = this->active;
	this->active = this->passive;
	this->passive = buffer;

	return false;
}

std::vector<std::array<int64_t, 2>>	CheckersBoard::get_legal_moves(int player) {
	std::vector<std::array<int64_t, 2>>	result;
	int 								buffer;
	std::vector<int64_t>				moves;
	std::array<int64_t, 2>				move_buff;

	if (player != this->active) {
		buffer = this->active;
		this->active = this->passive;
		this->passive = buffer;

		moves = this->_get_moves();
		for (auto i = moves.begin(); i != moves.end(); ++i) {
			move_buff = {*i, this->_get_move_direction(*i, this->active)};
			result.push_back(move_buff);
		}

		buffer = this->active;
		this->active = this->passive;
		this->passive = buffer;
	}
	else {
		moves = this->_get_moves();
		for (auto i = moves.begin(); i != moves.end(); ++i) {
			move_buff = {*i, this->_get_move_direction(*i, this->active)};
			result.push_back(move_buff);
		}
	}
	return result;
}

bool				CheckersBoard::is_over() {
	return (this->_get_moves().size() == 0);
}

CheckersBoard*		CheckersBoard::clone() {
	CheckersBoard *B = new CheckersBoard();

    B->backward = this->backward;
    B->forward = this->forward;
    B->pieces = this->pieces;
    B->empty = this->empty;

	B->active = this->active;
	B->passive = this->passive;

    B->jump = this->jump;
    B->mandatory_jumps = this->mandatory_jumps;

    return (B);
}

std::array<std::array<int, 8>, 8>	CheckersBoard::get_true_state() {
	return (this->get_observation(BLACK));
}

std::array<std::array<int, 8>, 8>	CheckersBoard::get_observation(int player) {
	std::array<std::array<int, 8>, 8>	board;
	int		x;
	int		y;
	int		buff;
	int64_t bin_black_pawn;
	int64_t bin_black_king;
	int64_t bin_white_pawn;
	int64_t bin_white_king;

	for (int i = 0; i < 8; i++)
		board[i].fill(0);
	bin_black_pawn = this->forward[BLACK];
    bin_black_king = this->backward[BLACK];
    bin_white_pawn = this->backward[WHITE];
    bin_white_king = this->forward[WHITE];

    if (player == BLACK){
        for (int i = 0; i < 35; i++) {
            if (((bin_black_king >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[y][x] = 3;
            } else if (((bin_white_king >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[y][x] = -3;
            } else if (((bin_black_pawn >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[y][x] = 1;
            } else if (((bin_white_pawn >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[y][x] = -1;
            }
        }
    } else {
        for (int i = 0; i < 35; i++) {
            if (((bin_black_king >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[7 - y][7 - x] = -3;
            } else if (((bin_white_king >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[7 - y][7 - x] = 3;
            } else if (((bin_black_pawn >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[7 - y][7 - x] = -1;
            } else if (((bin_white_pawn >> i) & 1) == 1) {
                buff = (1+i-i/9)-1;
                x = 6-(buff)%4*2+((buff)/4)%2;
                y = 7-(buff)/4;
                board[7 - y][7 - x] = 1;
            }
        }
    }
    return (board);
}

std::array<std::array<int, 8>, 8>	CheckersBoard::get_state_matrix_own(int player) {
    std::array<std::array<int, 8>, 8> observation = this->get_observation(player);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (observation[y][x] < 0)
                observation[y][x] = 0;
        }
    }
    return (observation);
}

std::array<std::array<int, 8>, 8>	CheckersBoard::get_state_matrix_enemy(int player) {
    std::array<std::array<int, 8>, 8> observation = this->get_observation(player);

    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            if (observation[y][x] > 0)
                observation[y][x] = 0;
        }
    }
    return (observation);
}

std::string			CheckersBoard::get_true_state_str() {
	std::array<std::array<int, 8>, 8> observation = this->get_true_state();
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

std::string			CheckersBoard::get_state_str(int player) {
	std::array<std::array<int, 8>, 8> observation = this->get_observation(player);
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

int					CheckersBoard::get_current_player() {
	return this->active;
}

void				CheckersBoard::set_current_player(int player) {
    int 				buffer;

    if (player != this->active)
    {
    	buffer = this->active;
		this->active = this->passive;
		this->passive = buffer;
    }
}

bool				CheckersBoard::is_draw() {
	return (false);
}

int64_t	    		CheckersBoard::_right_forward() {
	return ((this->empty >> 4) & this->forward[this->active]);
}

int64_t	    		CheckersBoard::_left_forward() {
	return ((this->empty >> 5) & this->forward[this->active]);
}

int64_t	    		CheckersBoard::_right_backward() {
	return ((this->empty << 4) & this->backward[this->active]);
}

int64_t	    		CheckersBoard::_left_backward() {
	return ((this->empty << 5) & this->backward[this->active]);
}

int64_t	    		CheckersBoard::_right_forward_jumps() {
	return ((this->empty >> 8) & (this->pieces[this->passive] >> 4) & this->forward[this->active]);
}

int64_t	    		CheckersBoard::_left_forward_jumps() {
	return ((this->empty >> 10) & (this->pieces[this->passive] >> 5) & this->forward[this->active]);
}

int64_t	    		CheckersBoard::_right_backward_jumps() {
	return ((this->empty << 8) & (this->pieces[this->passive] << 4) & this->backward[this->active]);
}

int64_t	    		CheckersBoard::_left_backward_jumps() {
	return ((this->empty << 10) & (this->pieces[this->passive] << 5) & this->backward[this->active]);
}


int64_t	    		CheckersBoard::_get_move_direction(int64_t move, int player) {
	if (move < 0)
		move = -move;
	return (this->pieces[player] < (this->pieces[player] ^ move));
}

std::vector<int64_t>		CheckersBoard::_get_moves() {
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
	if (this->jump) {
		return (this->mandatory_jumps);
	}
	// Next check if there are jumps
	jumps = this->_get_jumps();
	if (jumps.size() != 0) {
		return (jumps);
	} else {
		rf = this->_right_forward();
		lf = this->_left_forward();
		rb = this->_right_backward();
		lb = this->_left_backward();

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
			if ((buff & 1) == 1) {
				moves.push_back((static_cast<int64_t>(0x11) << (i - 4)));
			}
		}

		buff = static_cast<uint64_t>(lb);
		for(int i = 0; buff > 0; buff = (buff >> 1), i++) {
			if ((buff & 1) == 1) {
				moves.push_back((static_cast<int64_t>(0x21) << (i - 5)));
			}
		}
		return (moves);
	}
}


// moves = [0x11 << i for (i, bit) in enumerate(rf) if bit == '1']
// moves += [0x21 << i for (i, bit) in enumerate(lf) if bit == '1']
// moves += [0x11 << i - 4 for (i, bit) in enumerate(rb) if bit == '1']
// moves += [0x21 << i - 5 for (i, bit) in enumerate(lb) if bit == '1']

std::vector<int64_t>		CheckersBoard::_get_jumps() {
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

	rfj = this->_right_forward_jumps();
	lfj = this->_left_forward_jumps();
	rbj = this->_right_backward_jumps();
	lbj = this->_left_backward_jumps();
	
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

std::vector<int64_t>		CheckersBoard::_jumps_from(int64_t piece) {
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

    if (this->active == BLACK) {
        rfj = ((this->empty >> 8) & (this->pieces[this->passive] >> 4) & piece);
        lfj = ((this->empty >> 10) & (this->pieces[this->passive] >> 5) & piece);
        if (piece & this->backward[this->active]) { // piece at square is a king
            rbj = ((this->empty << 8) & (this->pieces[this->passive] << 4) & piece);
            lbj = ((this->empty << 10) & (this->pieces[this->passive] << 5) & piece);
        } else {
            rbj = 0;
            lbj = 0;
        }
    }
    else {
        rbj = ((this->empty << 8) & (this->pieces[this->passive] << 4) & piece);
        lbj = ((this->empty << 10) & (this->pieces[this->passive] << 5) & piece);
        if (piece & this->forward[this->active]) { // piece at square is a king
            rfj = ((this->empty >> 8) & (this->pieces[this->passive] >> 4) & piece);
            lfj = ((this->empty >> 10) & (this->pieces[this->passive] >> 5) & piece);
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

