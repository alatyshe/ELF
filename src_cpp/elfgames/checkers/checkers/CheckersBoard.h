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
#include <iomanip>
#include <utility>

// checkers
#include "hash_all_moves.h"
#include "elf/debug/debug.h"

// специальные хода
# define M_INVALID 171

# define MAX_CHECKERS_HISTORY 1
# define CHECKERS_BOARD_SIZE 8
# define TOTAL_PLAYERS 2

// Количество фич, которые мы подаем на нейронку
// (фишки врага, фишки наши, дамки врага, дамки наши, черные ходят, белые ходят)
constexpr uint64_t CHECKERS_NUM_FEATURES = 6;
// Количество всех возможных actions(для американских шашек это 170)
constexpr uint64_t TOTAL_NUM_ACTIONS = 170;
// Max move for game
constexpr int TOTAL_MAX_MOVE = 250;

constexpr int REPEAT_MOVE = 4;

// индекс нашего action;
typedef unsigned short	Coord;

#define BLACK_PLAYER 0
#define WHITE_PLAYER 1

#define BLACK_KING 2
#define WHITE_KING 3

// маска для заполнения поля(тк поле у нас храниться в 6 int64_t)
#define UNUSED_BITS 0b100000000100000000100000000100000000
#define MASK 0b111111111111111111111111111111111111

typedef struct 
{	
	// width
	int board_n;
	// just board
	std::array<int64_t, 2> forward;
	std::array<int64_t, 2> backward;
	std::array<int64_t, 2> pieces;
	int64_t empty;

	// Активный игрок
	int active;
	int passive;
	// есть ли возможность побить
	int jump;

	// последний шаг
	int _last_move;
	// общее количество шагов
	int _ply;

	// последный шаг для игроков
	std::array<int64_t, 2> _last_move_black;
	std::array<int64_t, 2> _last_move_white;
	bool _remove_step_black;
	bool _remove_step_white;
	int _black_repeats_step;
	int _white_repeats_step;
} CheckersBoard;




bool CheckersTryPlay(CheckersBoard board, Coord c);
bool CheckersPlay(CheckersBoard *board, int64_t action);
bool CheckersIsOver(CheckersBoard board);
float CheckersEvalBoard(CheckersBoard board, int player);

void ClearBoard(CheckersBoard *board);
void CheckersCopyBoard(CheckersBoard* dst, const CheckersBoard* src);

std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(CheckersBoard board, int player);
std::vector<std::array<int64_t, 2>> GetValidMovesNumberAndDirection(CheckersBoard board, int player);


// Доска в представлении массива 8x8
std::array<std::array<int, 8>, 8> GetTrueState(const CheckersBoard board);
std::array<std::array<int, 8>, 8> GetObservation(const CheckersBoard board, int player);
std::string GetTrueStateStr(const CheckersBoard board);
// std::string get_state_str(const CheckersBoard *board, int player);


// just logic board
int64_t _right_forward(CheckersBoard board);
int64_t _left_forward(CheckersBoard board);
int64_t _right_backward(CheckersBoard board);
int64_t _left_backward(CheckersBoard board);
int64_t _right_forward_jumps(CheckersBoard board);
int64_t _left_forward_jumps(CheckersBoard board);
int64_t _right_backward_jumps(CheckersBoard board);
int64_t _left_backward_jumps(CheckersBoard board);
int64_t _get_move_direction(CheckersBoard board, int64_t move, int player);
std::vector<int64_t> _get_moves(CheckersBoard board);
std::vector<int64_t> _get_jumps(CheckersBoard board);
std::vector<int64_t> _jumps_from(CheckersBoard board, int64_t piece);


