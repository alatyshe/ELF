#pragma once

#include <bitset>

#include <queue>
#include <vector>
#include <memory.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <cmath>



// ugolki
#include "HashAllMoves.h"

#define BLACK_C   "\u001b[30m"
#define RED_C     "\u001b[31m"
#define GREEN_C   "\u001b[32m"
#define YELLOW_C  "\u001b[33m"
#define BLUE_C    "\u001b[34m"
#define MAGENTA_C "\u001b[35m"
#define CYAN_C    "\u001b[36m"
#define WHITE_C   "\u001b[37m"

#define BLACK_B   "\u001b[30;1m"
#define RED_B     "\u001b[31;1m"
#define GREEN_B   "\u001b[32;1m"
#define YELLOW_B  "\u001b[33;1m"
#define BLUE_B    "\u001b[34;1m"
#define MAGENTA_B "\u001b[35;1m"
#define CYAN_B    "\u001b[36;1m"
#define WHITE_B   "\u001b[37;1m"

#define COLOR_END "\u001b[0m"


// special moves
# define M_INVALID 419

# define MAX_HISTORY 4
# define BOARD_SIZE 8
# define TOTAL_PLAYERS 2

// number of layers
// (our pawns + enemy pawns) 
//            * MAX_CHECKERS_HISTORY + black move + white move
constexpr uint64_t NUM_FEATURES = 4 * MAX_HISTORY;

constexpr uint64_t TOTAL_NUM_ACTIONS = 418;

// Max move for game
constexpr int TOTAL_MAX_MOVE = 500;


// action index;
typedef unsigned short  Coord;

// Ugolki
#define BLACK_PLAYER 0
#define WHITE_PLAYER 1

#define WHITE_BASE 0xE0E0E00000000000UL
#define BLACK_BASE 0x0000000000070707UL

typedef struct {
  // Ugolki
  int active;
  int passive;

  uint64_t jump_action;
  std::array<uint64_t, 2> pieces;

  int white_win;
  int black_win;

  uint64_t _last_move;
  // move num
  int _ply;
} GameBoard;

bool TryPlay(GameBoard board, Coord c);
bool Play(GameBoard *board, int action_index);
bool IsOver(GameBoard board);

void ClearBoard(GameBoard *board);
bool CompareBoards(GameBoard b1, GameBoard b2);
void CopyBoard(GameBoard* dst, const GameBoard* src);

std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(GameBoard board);

std::array<std::array<int, 8>, 8> GetTrueObservation(const GameBoard board);
std::array<std::array<int, 8>, 8> GetObservation(const GameBoard board, int player);
std::string GetTrueObservationStr(const GameBoard board);
std::vector<std::array<uint64_t, 2>> get_legal_moves(GameBoard board);

// board logic
uint64_t _ugolki_right(GameBoard board, uint64_t pieces);
uint64_t _ugolki_left(GameBoard board, uint64_t pieces);
uint64_t _ugolki_forward(GameBoard board, uint64_t pieces);
uint64_t _ugolki_backward(GameBoard board, uint64_t pieces);
uint64_t _ugolki_right_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move=0);
uint64_t _ugolki_left_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move=0);
uint64_t _ugolki_forward_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move=0);
uint64_t _ugolki_backward_jumps(GameBoard board, uint64_t pieces, uint64_t invalid_move=0);
uint64_t _ugolki_get_move_direction(uint64_t move, uint64_t pieces);

std::vector<std::array<uint64_t, 2>> _get_all_moves(GameBoard board, uint64_t pieces);
std::vector<std::array<uint64_t, 2>> _jumps_from(GameBoard board, uint64_t pieces, uint64_t jump_action);

