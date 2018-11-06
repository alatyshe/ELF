/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <memory.h>
#include <stdio.h>
#include "common.h"
#include "elf/debug/debug.h"

#define STAR_ON9(i, j) \
  ((((i) == 2 || (i) == 6) && ((j) == 2 || (j) == 6)) || (i == 4 && j == 4))
#define BOARD9_PROMPT "A B C D E F G H J"

#define STAR_ON STAR_ON9
#define BOARD_PROMPT BOARD9_PROMPT
#define __MACRO_BOARD_SIZE 9


constexpr int BOARD_SIZE = __MACRO_BOARD_SIZE;
constexpr int BOARD_MARGIN = 1;
constexpr int BOARD_EXPAND_SIZE = BOARD_SIZE + 2;
constexpr int NUM_INTERSECTION = BOARD_SIZE * BOARD_SIZE;

#include "hash_num.h"

typedef unsigned char Status;
typedef unsigned char ShowChoice;
#define SHOW_NONE 0
#define SHOW_LAST_MOVE 1
#define SHOW_ROWS 2
#define SHOW_COLS 3
#define SHOW_ALL 4
#define SHOW_ALL_ROWS_COLS 5

typedef struct {
  // Group id of which the stone belongs to.
  Stone color;
  // id == 0 means that it is an empty intersection. id == MAX_GROUP means it is
  // the border.
  unsigned char id;
  // The next location on the board.
  Coord next;
  // History information, what is the last time that the stone has placed.
  unsigned short last_placed;
} Info;






typedef struct {
  Stone color;
  Coord start;
  short stones;
  short liberties;
} Group;






typedef struct {
  Coord c;
  Stone player;
  short ids[4];
  Stone colors[4];
  short group_liberties[4];
  short liberty;
} GroupId4;





// How many live groups can possibly be there in a game?
// We use 173 so that sizeof(MBoard) <= 4096. This is important for atomic data
// transmission using pipe.
#define MAX_GROUP 173

// Maximum possible value of coords.
constexpr int BOUND_COORD = BOARD_EXPAND_SIZE * BOARD_EXPAND_SIZE;

// Board
typedef struct {
  // Board
  Info _infos[BOARD_EXPAND_SIZE * BOARD_EXPAND_SIZE];

  typedef unsigned char Bits[BOARD_EXPAND_SIZE * BOARD_EXPAND_SIZE / 4 + 1];
  Bits _bits;
  uint64_t _hash;

  // Group info
  Group _groups[MAX_GROUP];
  // Number of groups, including group 0 (empty intersection). So for empty
  // board, _num_groups == 1.
  short _num_groups;

  // After each play, the number of possible empty groups cannot exceed 4.
  // unsigned short _empty_group_ids[4];
  // int _next_empty_group;

  // Capture info
  short _b_cap;
  short _w_cap;

  // Passes in rollout (B-W) for score parity
  short _rollout_passes;

  // Last played location
  Coord _last_move;
  Coord _last_move2;
  Coord _last_move3;
  Coord _last_move4;

  // These record the group ids that change during play. (Useful for external
  // function to modify their records accordingly).
  // _removed_group_ids recorded all the group ids that are to be killed,
  // _num_group_removed is the number of groups to be replaced, it is always in
  // [0, 4].
  unsigned char _removed_group_ids[4];
  unsigned char _num_group_removed;

  // Simple ko point.
  // _ko_age. _ko_age == 0 mean the ko is recent and cannot be violated. ko_age_
  // increases when play continues,
  // until it is refreshed by a new ko.
  unsigned short _ko_age;
  Coord _simple_ko;
  Stone _simple_ko_color;

  // Next player.
  Stone _next_player;

  // The current ply number, it will be increase after each play.
  // The initial ply number is 1.
  short _ply;
  // uint64_t hash for the current board situatons. If we want to enable it,
  // then make MAX_GROUP smaller.
  // uint64_t hash;
} Board;

#define HAS_STONE(s) (((s) == S_BLACK) || ((s) == S_WHITE))
#define OPPONENT(p) ((Stone)(S_WHITE + S_BLACK - (int)(p)))
#define G_ONBOARD(id) ((id) != MAX_GROUP)
#define G_EMPTY(id) ((id) == 0)
#define EMPTY(s) ((s) == S_EMPTY)
#define EXTENDOFFSETXY(x, y) ((y)*BOARD_EXPAND_SIZE + (x))
// Coordinate/Move
// X first, then y.
#define X(c) ((c) % BOARD_EXPAND_SIZE - 1)
#define Y(c) ((c) / BOARD_EXPAND_SIZE - 1)


#define ONBOARD(s) ((s) != S_OFF_BOARD)
#define ON_BOARD(x, y) \
  ((x) >= 0 && (x) < BOARD_SIZE && (y) >= 0 && (y) < BOARD_SIZE)

// Warning, the export offset is different from internal representation
// Internal representation: y * stride + x
// External representation: x * stride + y
#define EXPORT_OFFSET_XY(x, y) ((x)*BOARD_SIZE + (y))

#define EXPORT_OFFSET(c) ((X(c)) * BOARD_SIZE + (Y(c)))
#define EXPORT_X(a) ((a) / BOARD_SIZE)

#define EXPORT_Y(a) ((a) % BOARD_SIZE)
#define OFFSETXY(x, y) \
  (((y) + BOARD_MARGIN) * BOARD_EXPAND_SIZE + (x) + BOARD_MARGIN)

// Faster access to neighbors.
// L/R means x-1/x+1
// T/B means y-1/y+1
#define GO_L(c) ((c)-1)
#define GO_R(c) ((c) + 1)
#define GO_T(c) ((c)-BOARD_EXPAND_SIZE)
#define GO_B(c) ((c) + BOARD_EXPAND_SIZE)
#define GO_LT(c) ((c)-1 - BOARD_EXPAND_SIZE)
#define GO_LB(c) ((c)-1 + BOARD_EXPAND_SIZE)
#define GO_RT(c) ((c) + 1 - BOARD_EXPAND_SIZE)
#define GO_RB(c) ((c) + 1 + BOARD_EXPAND_SIZE)
#define GO_LL(c) ((c)-2)
#define GO_RR(c) ((c) + 2)
#define GO_TT(c) ((c)-2 * BOARD_EXPAND_SIZE)
#define GO_BB(c) ((c) + 2 * BOARD_EXPAND_SIZE)


// Left, top, right, bottom
static const int delta4[4] = {-1, -BOARD_EXPAND_SIZE, +1, BOARD_EXPAND_SIZE};


// Loop through the group link table
#define TRAVERSE(b, id, c) \
  for (Coord c = b->_groups[id].start; c != 0; c = b->_infos[c].next) {
#define ENDTRAVERSE }

// Local check
#define FOR4(c, ii, cc)            \
  for (int ii = 0; ii < 4; ++ii) { \
    Coord cc = c + delta4[ii];
#define ENDFOR4 }


// Zero based.
extern inline Coord getCoord(int x, int y) {
  return OFFSETXY(x, y);
}

void clearBoard(Board* board);
void copyBoard(Board* dst, const Board* src);
// Return true if the move is valid and can be played, if so, properly set up
// ids
// Otherwise return false.
bool TryPlay(const Board* board, int x, int y, Stone player, GroupId4* ids);
// Simple version of it.
bool TryPlay2(const Board* board, Coord m, GroupId4* ids);

// Actually play the game. If return true, then the game ended (either by PASS +
// PASS or by RESIGN)
bool Play(Board* board, const GroupId4* ids);

// Place handicap stone.
bool PlaceHandicap(Board* board, int x, int y, Stone player);
void showBoard2Buf(const Board* board, ShowChoice choice, char* buf);
void showBoard(const Board* board, ShowChoice choice);

// Get the current simple ko location.
Coord getSimpleKoLocation(const Board* board, Stone* player);

// Check if the game has ended
bool isGameEnd(const Board* board);
bool isBitsEqual(const Board::Bits bits1, const Board::Bits bits2);
void copyBits(Board::Bits bits_dst, const Board::Bits bits_src);

// DEAD/ALIVE/UNKNOWN are bits superimposed to the existing stone.
#define S_UNKNOWN 4

// Some utility functions.
char* get_move_str(Coord m, Stone player, char* buf);
