#pragma once

#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <cmath>
#include <bitset>

// elf
#include "elf/logging/IndexedLoggerFactory.h"

// checkers
#include "hash_all_moves.h"


constexpr uint64_t TOTAL_NUM_ACTIONS = 170;
// максимальное Количество ходов за игру
// (устанавливаем мы, чтобы нейронка не играла бесконечно)
constexpr int TOTAL_MAX_MOVE = 700;

// typedef int64_t Coord;
// typedef unsigned char Stone;

#define BLACK_PLAYER 0 
#define WHITE_PLAYER 1

#define BLACK 0
#define WHITE 1
#define BLACK_KING 2
#define WHITE_KING 3

#define UNUSED_BITS 0b100000000100000000100000000100000000
#define MASK        0b111111111111111111111111111111111111

# define BLACK_PLAYER 	0 
# define WHITE_PLAYER 	1
# define ALL_ACTIONS 	170
# define BOARD_SIZE		8
# define TOTAL_PLAYERS	2

# define PLAYER_MAPPING (n) ((n) == 0 ? BLACK_PLAYER : WHITE_PLAYER)

typedef struct 
{
	// общее количество шагов
	int						_total_moves;
	
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
	std::vector<int64_t>	mandatory_jumps;

	// последный шаг для игроков
	int64_t 			_last_move_green;
	int64_t 			_last_move_red;
} CheckersBoard;




void					clearBoard(CheckersBoard *board);
bool					make_move(CheckersBoard *board, int64_t action);
std::vector<std::array<int64_t, 2>>	get_legal_moves(CheckersBoard board, int player);
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




















std::string				player2str(int player);

