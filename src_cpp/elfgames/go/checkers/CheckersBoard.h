#pragma once

#include <random>
#include <deque>
#include <queue>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>
#include <bitset>
#include <memory.h>
#include <stdio.h>


// checkers
#include "hash_all_moves.h"
#include "elf/debug/debug.h"

#define M_INVALID 		171

constexpr uint64_t CHECKERS_NUM_FEATURES = 6;
constexpr uint64_t TOTAL_NUM_ACTIONS = 170;
// максимальное Количество ходов за игру
// (устанавливаем мы, чтобы нейронка не играла бесконечно)
constexpr int TOTAL_MAX_MOVE = 500;

// typedef int64_t Coord;
typedef unsigned short 	Coord;

#define BLACK_PLAYER 	0
#define WHITE_PLAYER 	1

#define BLACK_KING 		2
#define WHITE_KING 		3

#define UNUSED_BITS 	0b100000000100000000100000000100000000
#define MASK        	0b111111111111111111111111111111111111

# define BLACK_PLAYER 			0 
# define WHITE_PLAYER 			1
# define ALL_ACTIONS 			170
# define MAX_CHECKERS_HISTORY	1
# define CHECKERS_BOARD_SIZE	8
# define TOTAL_PLAYERS			2

// # define PLAYER_MAPPING (n) ((n) == 0 ? BLACK_PLAYER : WHITE_PLAYER)

typedef struct 
{	
	// ширина доски
	int 					board_n;
	// логика доски
	std::array<int64_t, 2>	forward;
	std::array<int64_t, 2>	backward;
	std::array<int64_t, 2>	pieces;
	int64_t					empty;

	// Активный игрок
	int						active;
	int						passive;
	// есть ли возможность побить
	int						jump;
	// все возможные ходы битья 
	// std::vector<int64_t>	mandatory_jumps;

	int 					_last_move;
	// общее количество шагов
	int 					_ply;

	// последный шаг для игроков
	int64_t 				_last_move_green;
	int64_t 				_last_move_red;
} CheckersBoard;




void					ClearBoard(CheckersBoard *board);
void					CheckersCopyBoard(CheckersBoard* dst, const CheckersBoard* src);
bool					CheckersPlay(CheckersBoard *board, int64_t action);

std::array<int, ALL_ACTIONS>		GetValidMovesBinary(CheckersBoard board, int player);
std::vector<std::array<int64_t, 2>>	GetValidMovesNumberAndDirection(CheckersBoard board, int player);
bool					CheckersTryPlay(CheckersBoard board, Coord c);

bool					is_over(CheckersBoard board);

std::array<std::array<int, 8>, 8>	get_true_state(const CheckersBoard board);
std::array<std::array<int, 8>, 8>	get_observation(const CheckersBoard board, int player);
// std::array<std::array<int, 8>, 8>	get_state_matrix_own(const CheckersBoard board, int player);
// std::array<std::array<int, 8>, 8>	get_state_matrix_enemy(const CheckersBoard board, int player);
// // // 
std::string				get_true_state_str(const CheckersBoard board);
// std::string				get_state_str(const CheckersBoard *board, int player);

// int						get_current_player(CheckersBoard board);
// void					set_current_player(CheckersBoard board, int player);
// bool					is_draw(CheckersBoard board);











// // just logic
int64_t					_right_forward(CheckersBoard board);
int64_t					_left_forward(CheckersBoard board);
int64_t					_right_backward(CheckersBoard board);
int64_t					_left_backward(CheckersBoard board);
int64_t					_right_forward_jumps(CheckersBoard board);
int64_t					_left_forward_jumps(CheckersBoard board);
int64_t					_right_backward_jumps(CheckersBoard board);
int64_t					_left_backward_jumps(CheckersBoard board);
int64_t					_get_move_direction(CheckersBoard board, int64_t move, int player);
std::vector<int64_t>	_get_moves(CheckersBoard board);
std::vector<int64_t>	_get_jumps(CheckersBoard board);
std::vector<int64_t>	_jumps_from(CheckersBoard board, int64_t piece);




















// std::string				player2str(int player);

