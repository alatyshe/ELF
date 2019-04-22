#pragma once 

#include <queue>
#include <memory.h>
# include <iostream>
# include <sstream>
# include <string>
# include <vector>
# include <array>
# include <bitset>
# include <iomanip>
# include <cstdlib>

#include "HashAllMoves.h"

#define BOARD_SIZE 8

#define EMPTY 0
#define TEMP 2
#define WHITE_PLAYER  1
#define BLACK_PLAYER  -1


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

constexpr uint64_t NUM_FEATURES = 2;
constexpr uint64_t TOTAL_NUM_ACTIONS = 418;
// // Max move for game
constexpr int TOTAL_MAX_MOVE = 150;

# define M_INVALID 419

typedef unsigned short Coord;


typedef struct {
  int    board[8][8];
  int    current_player;
  bool   game_ended;

  int    jump_y;
  int    jump_x;

  bool   white_must_leave_base;
  bool   black_must_leave_base;

  // last move id
  int    _last_move;
  // move number
  int    _ply;
} CheckersBoard;



// Инициализация состояния игры
bool    CheckersTryPlay(CheckersBoard board, int64_t c);
void    CheckersPlay(CheckersBoard *board, int64_t action_index);
bool    CheckersIsOver(CheckersBoard board);

void    ClearBoard(CheckersBoard *board);
void    CheckersCopyBoard(CheckersBoard *dst, const CheckersBoard *src);

std::array<int, TOTAL_NUM_ACTIONS> GetValidMovesBinary(CheckersBoard board);
std::array<std::array<int, 8>, 8>  GetObservation(const CheckersBoard board, int player);
std::array<std::array<int, 8>, 8>  GetTrueObservation(const CheckersBoard board);
std::string                        GetTrueObservationStr(const CheckersBoard board);


std::vector<std::array<int, 2>>   _getJump(CheckersBoard board, int y, int  x);
std::vector<std::array<int, 2>>   _getJumps(CheckersBoard board);
std::vector<std::array<int, 2>>   _getMoves(CheckersBoard board);
std::vector<std::array<int, 2>>   _getAllMoves(CheckersBoard board);


