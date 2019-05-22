#pragma once 

#include <iostream>
#include <sstream>
#include <queue>
#include <vector>
#include <memory.h>
#include <iomanip>

#include "HashAllMoves.h"

#define UP 1
#define DOWN -1
#define LEFT -1
#define RIGHT 1

#define EMPTY 0

#define WHITE_PLAYER  1
#define WHITE_PAWN    1
#define WHITE_KING    3

#define BLACK_PLAYER  -1
#define BLACK_PAWN    -1
#define BLACK_KING    -3


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
# define M_INVALID 281

# define CHECKERS_BOARD_SIZE 8

// number of layers
// (our pawns, our kings, enemy pawns, enemy kings, black move, white move)
constexpr uint64_t CHECKERS_NUM_FEATURES = 6;
// for american checkers total num of actions for board will be 280
constexpr uint64_t TOTAL_NUM_ACTIONS = 280;
// Max move for game
constexpr int TOTAL_MAX_MOVE = 250;
// max num of repeat moves for board
constexpr int REPEAT_MOVE = 4;

// action index;
typedef unsigned short  Coord;


typedef struct {
  int   board[8][8];
  int   current_player;
  bool  game_ended;

  int   next_bit_y;
  int   next_bit_x;

  // last move id
  int   _last_move;
  // move number
  int   _ply;
} CheckersBoard;




bool  CheckersTryPlay(CheckersBoard board, Coord action);
bool  CheckersIsOver(CheckersBoard board);
void  CheckersPlay(CheckersBoard *board, Coord action_index);
void  ClearBoard(CheckersBoard *board);

void  CheckersCopyBoard(CheckersBoard* dst, const CheckersBoard* src);

std::vector<std::array<int, 2>> getAllMoves(CheckersBoard board);

std::array<int, TOTAL_NUM_ACTIONS>  GetValidMovesBinary(CheckersBoard board);
std::array<std::array<int, 8>, 8>   GetTrueObservation(const CheckersBoard board);
std::array<std::array<int, 8>, 8>   GetObservation(const CheckersBoard board, int player);
std::string GetTrueObservationStr(const CheckersBoard board);



bool                            _coordOverflow(int num);
std::vector<std::array<int, 2>> _pawnMoves(CheckersBoard board, int y, int x);

std::vector<std::array<int, 2>> _kingMoves(CheckersBoard board, int y, int x);
bool                            _kingJumpCheck(CheckersBoard board, int y, int x, int dir_y, int dir_x);
std::vector<std::array<int, 2>> _kingJumpInDirection(CheckersBoard board, int y, int x, int dir_y, int dir_x);
std::vector<std::array<int, 2>> _kingJumps(CheckersBoard board, int y, int x);

std::vector<std::array<int, 2>> _pawnJumps(CheckersBoard board, int y, int x);
std::array<int, 2>              _pawnJumpInDirection(CheckersBoard board, int y, int x, int dir_y, int dir_x);


void                            _tryConvertIntoKing(CheckersBoard *board, int y, int x);







