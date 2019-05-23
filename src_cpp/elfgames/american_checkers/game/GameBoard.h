#pragma once

#include <queue>
#include <vector>
#include <memory.h>
#include <iomanip>

// checkers
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
# define M_INVALID 171

# define MAX_CHECKERS_HISTORY 6
# define CHECKERS_BOARD_SIZE 8
# define TOTAL_PLAYERS 2

// number of layers
// (our pawns + our kings + enemy pawns + enemy kings) 
//            * MAX_CHECKERS_HISTORY + black move + white move
constexpr uint64_t CHECKERS_NUM_FEATURES = 6 * MAX_CHECKERS_HISTORY;
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
#define MASK        0b111111111111111111111111111111111111

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
} GameBoard;

bool CheckersTryPlay(GameBoard board, Coord c);
bool CheckersPlay(GameBoard *board, int64_t action);
bool CheckersIsOver(GameBoard board);

void ClearBoard(GameBoard *board);
void GameCopyBoard(GameBoard* dst, const GameBoard* src);

std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(GameBoard board);
std::vector<std::array<int64_t, 2>> GetValidMovesNumberAndDirection(GameBoard board, int player);

std::array<std::array<int, 8>, 8> GetTrueState(const GameBoard board);
std::array<std::array<int, 8>, 8> GetObservation(const GameBoard board, int player);
std::string GetTrueStateStr(const GameBoard board);
// std::string get_state_str(const GameBoard *board, int player);

// board logic
int64_t _right_forward(GameBoard board);
int64_t _left_forward(GameBoard board);
int64_t _right_backward(GameBoard board);
int64_t _left_backward(GameBoard board);
int64_t _right_forward_jumps(GameBoard board);
int64_t _left_forward_jumps(GameBoard board);
int64_t _right_backward_jumps(GameBoard board);
int64_t _left_backward_jumps(GameBoard board);
int64_t _get_move_direction(GameBoard board, int64_t move, int player);
std::vector<int64_t> _get_moves(GameBoard board);
std::vector<int64_t> _get_jumps(GameBoard board);
std::vector<int64_t> _jumps_from(GameBoard board, int64_t piece);

