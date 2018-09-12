#pragma once

#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>
#include <bitset>


#define BLACK_PLAYER 0 
#define WHITE_PLAYER 1

#define BLACK 0
#define WHITE 1
#define EMPTY -1
#define BLACK_KING 2
#define WHITE_KING 3

#define UNUSED_BITS 0b100000000100000000100000000100000000
#define MASK        0b111111111111111111111111111111111111

class CheckersBoard {
// private:

	

public:
	int 					board_n;
	std::array<int64_t, 2>	forward;
	std::array<int64_t, 2>	backward;
	std::array<int64_t, 2>	pieces;
	int64_t					empty;

	int						active;
	int						passive;
	int						jump;
	std::vector<int64_t>	mandatory_jumps;


	CheckersBoard();
	// CheckersGame::CheckersGame(const CheckersGame& s);
	~CheckersBoard();

	void					new_game();
    bool					make_move(std::array<int64_t, 2> action);
    std::vector<std::array<int64_t, 2>>	get_legal_moves(int player);
	bool					is_over();
	CheckersBoard*			clone();

	std::array<std::array<int, 8>, 8>	get_true_state();
	std::array<std::array<int, 8>, 8>	get_observation(int player);
	std::array<std::array<int, 8>, 8>	get_state_matrix_own(int player);
	std::array<std::array<int, 8>, 8>	get_state_matrix_enemy(int player);
	// // 
	std::string				get_true_state_str();
	std::string				get_state_str(int player);
	int						get_current_player();
	void					set_current_player(int player);
	bool					is_draw();

private:
	int64_t					_right_forward();
	int64_t					_left_forward();
	int64_t					_right_backward();
	int64_t					_left_backward();
	int64_t					_right_forward_jumps();
	int64_t					_left_forward_jumps();
	int64_t					_right_backward_jumps();
	int64_t					_left_backward_jumps();
	int64_t					_get_move_direction(int64_t move, int player);
	std::vector<int64_t>	_get_moves();
	std::vector<int64_t>	_get_jumps();
	std::vector<int64_t>	_jumps_from(int64_t piece);
};
