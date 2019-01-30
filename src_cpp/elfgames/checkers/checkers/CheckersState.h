#pragma once

// ELF
#include "elf/debug/debug.h"

// Checkers
#include "CheckersBoard.h"
#include "CheckersFeature.h"


class CheckersState {
 public:

	CheckersState() {
		reset();
	}
	
	CheckersState(const CheckersState& s)
			: _history(s._history),
				// _board_hash(s._board_hash),
				_moves(s._moves),
				_final_value(s._final_value),
				_has_final_value(s._has_final_value) {
		CheckersCopyBoard(&_board, &s._board);
	}

	bool forward(const Coord& c);
	bool checkMove(const Coord& c) const;

	void	setFinalValue(float final_value) {
		_final_value = final_value;
		_has_final_value = true;
	}

	float	getFinalValue() const {
		return _final_value;
	}

	bool	HasFinalValue() const {
		return _has_final_value;
	}

	void	reset();

	const CheckersBoard& board() const {
		return _board;
	}

	// Note that ply started from 1.
	bool	justStarted() const {
		return _board._ply == 1;
	}

	int		getPly() const {
		return _board._ply;
	}

	bool	terminated() const {
		return is_over(_board) || getPly() >= TOTAL_MAX_MOVE ;
	}

	int		lastMove() const {
		return _board._last_move;
	}

	int		nextPlayer() const {
		return _board.active;
	}

	bool	moves_since(size_t* next_move_number, std::vector<Coord>* moves) const {
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

	const std::vector<Coord>& getAllMoves() const {
		return _moves;
	}

	std::string getAllMovesString() const {
		std::stringstream ss;
		for (const Coord& c : _moves) {
			ss << "[" << c << "] ";
		}
		return ss.str();
	}

	std::string showBoard() const {
		std::string   res;

		res = get_true_state_str(_board);
		return std::string(res) + "\nLast move\t: " + std::to_string(lastMove()) +
				"\nCurrentPlayer\t: " + (this->nextPlayer() == BLACK_PLAYER ? 
					"\u001b[32mBlack" : "\u001b[31mWhite") + 
				"\u001b[0m\nmove num\t: " + std::to_string(_board._ply) + "\n";
	}

	float	evaluateGame() const {
		float final_score = 0.0;
		if (terminated()) {
			if (getPly() >= TOTAL_MAX_MOVE)
				final_score = -1;
			else
				final_score = this->nextPlayer() == BLACK_PLAYER ? -1.0 : 1.0;
		}
		return final_score;
	}

	// TODO: not a good design..
	// const std::deque<BoardHistory>& getHistory() const {
	//   return _history;
	// }

 protected:
	CheckersBoard                     _board;
	std::deque<CheckersBoardHistory>  _history;

 //  struct _BoardRecord {
 //    Board::Bits bits;
 //  };

 //  std::unordered_map<uint64_t, std::vector<_BoardRecord>> _board_hash;

	std::vector<Coord> _moves;
	float _final_value = 0.0;
	bool _has_final_value = false;
};



















