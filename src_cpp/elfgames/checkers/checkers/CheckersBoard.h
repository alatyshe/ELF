#pragma once

#include <queue>
#include <vector>
#include <memory.h>
#include <iomanip>

// checkers
#include "hash_all_moves.h"
#include "elf/debug/debug.h"

// специальные хода
# define M_INVALID 171

# define MAX_CHECKERS_HISTORY 1
# define CHECKERS_BOARD_SIZE 8
# define TOTAL_PLAYERS 2

// number of layers
// (our pawns, our kings, enemy pawns, enemy kings, black move, white move)
constexpr uint64_t CHECKERS_NUM_FEATURES = 6;
// for american checkers total num of actions for board will be 170
constexpr uint64_t TOTAL_NUM_ACTIONS = 170;
// Max move for game
constexpr int TOTAL_MAX_MOVE = 250;
// max num of repeat moves for board
constexpr int REPEAT_MOVE = 4;

// action index;
typedef unsigned short  Coord;

#define BLACK_PLAYER 0
#define WHITE_PLAYER 1

#define BLACK_KING 2
#define WHITE_KING 3

// mask for fill the board(board stores in 6 int64_t)
#define UNUSED_BITS 0b100000000100000000100000000100000000
#define MASK 0b111111111111111111111111111111111111

typedef struct {
  // board
  std::array<int64_t, 2> forward;
  std::array<int64_t, 2> backward;
  std::array<int64_t, 2> pieces;
  int64_t empty;

  // active player
  int active;
  int passive;
  // beat
  int jump;

  int _last_move;
  // move num
  int _ply;

  // last move for player(move, move direction)
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

// Board in 8x8 array
std::array<std::array<int, 8>, 8> GetTrueState(const CheckersBoard board);
std::array<std::array<int, 8>, 8> GetObservation(const CheckersBoard board, int player);
std::string GetTrueStateStr(const CheckersBoard board);
// std::string get_state_str(const CheckersBoard *board, int player);

// board logic
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

