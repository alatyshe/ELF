/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "board.h"
#include <iostream>
#include <vector>

#define myassert(p, text) \
  do {                    \
    if (!(p)) {           \
      printf((text));     \
    }                     \
  } while (0)
#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))


uint64_t transform_hash(uint64_t h, Stone s) {
  // display_debug_info("", __FUNCTION__, BLUE_B);
  switch (s) {
    case S_EMPTY:
    case S_OFF_BOARD:
      return 0;
    case S_BLACK:
      return h;
    case S_WHITE:
      return (h >> 32) | ((h & ((1ULL << 32) - 1)) << 32);
    default:
      return h;
  }
}

inline void set_color(Board* board, Coord c, Stone s) {
  // display_debug_info("", __FUNCTION__, BLUE_B);

  Stone old_s = board->_infos[c].color;
  board->_infos[c].color = s;

  unsigned char offset = ((c & 3) << 1);
  unsigned char mask = ~(3 << offset);
  board->_bits[c >> 2] &= mask;
  board->_bits[c >> 2] |= (s << offset);

  uint64_t h = _board_hash[c];

  board->_hash ^= transform_hash(h, old_s);
  board->_hash ^= transform_hash(h, s);
}

bool isBitsEqual(const Board::Bits bits1, const Board::Bits bits2) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  for (size_t i = 0; i < sizeof(Board::Bits) / sizeof(unsigned char); ++i) {
    if (bits1[i] != bits2[i])
      return false;
  }
  return true;
}

void copyBits(Board::Bits bits_dst, const Board::Bits bits_src) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  ::memcpy((void*)bits_dst, (const void*)bits_src, sizeof(Board::Bits));
}

// Functions..
void setAsBorder(Board* board, int /*side*/, int i1, int w, int j1, int h) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  for (int i = i1; i < i1 + w; i++) {
    for (int j = j1; j < j1 + h; ++j) {
      if (i < 0 || i >= BOARD_EXPAND_SIZE || j < 0 || j >= BOARD_EXPAND_SIZE) {
        printf("Fill: (%d, %d) is out of bound!", i, j);
      }
      Coord c = EXTENDOFFSETXY(i, j);
      set_color(board, c, S_OFF_BOARD);
      board->_infos[c].id = MAX_GROUP;
    }
  }
}

void clearBoard(Board* board) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // The initial hash is zero.
  memset((void*)board, 0, sizeof(Board));
  // Setup the offboard mark.
  setAsBorder(board, BOARD_EXPAND_SIZE, 0, BOARD_MARGIN, 0, BOARD_EXPAND_SIZE);
  setAsBorder(
      board,
      BOARD_EXPAND_SIZE,
      BOARD_SIZE + BOARD_MARGIN,
      BOARD_MARGIN,
      0,
      BOARD_EXPAND_SIZE);
  setAsBorder(board, BOARD_EXPAND_SIZE, 0, BOARD_EXPAND_SIZE, 0, BOARD_MARGIN);
  setAsBorder(
      board,
      BOARD_EXPAND_SIZE,
      0,
      BOARD_EXPAND_SIZE,
      BOARD_SIZE + BOARD_MARGIN,
      BOARD_MARGIN);
  board->_next_player = S_BLACK;
  board->_last_move = M_INVALID;
  board->_last_move2 = M_INVALID;
  board->_last_move3 = M_INVALID;
  board->_last_move4 = M_INVALID;
  board->_num_groups = 1;
  // The initial ply number is 1.
  board->_ply = 1;
}

bool PlaceHandicap(Board* board, int x, int y, Stone player) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // If the game has already started, return false.
  if (board->_ply > 1)
    return false;
  GroupId4 ids;
  if (TryPlay(board, x, y, player, &ids)) {
    Play(board, &ids);
    // Keep the board situations.
    board->_ply = 1;
    board->_last_move = M_INVALID;
    board->_last_move2 = M_INVALID;
    board->_last_move3 = M_INVALID;
    board->_last_move4 = M_INVALID;
    return true;
  }
  // Not a valid move.
  return false;
}

void copyBoard(Board* dst, const Board* src) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  myassert(dst, "dst cannot be nullptr");
  myassert(src, "src cannot be nullptr");
  memcpy(dst, src, sizeof(Board));
}

// board analysis, whether putting or removing this stone will yield a change in
// the liberty in the surrounding group,
// Also we could get the liberty of that stone as well.
static inline void
StoneLibertyAnalysis(const Board* board, Stone player, Coord c, GroupId4* ids) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  memset(ids, 0, sizeof(GroupId4));
  ids->c = c;
  ids->player = player;
  FOR4(c, i4, c4) {
    unsigned short group_id = board->_infos[c4].id;
    if (G_EMPTY(group_id)) {
      ids->liberty++;
      continue;
    }
    if (!G_ONBOARD(group_id))
      continue;

    // simple way to check duplicate group ids
    bool visited_before = false;
    // Unrolling.
    visited_before += (ids->ids[0] == group_id);
    visited_before += (ids->ids[1] == group_id);
    visited_before += (ids->ids[2] == group_id);
    if (visited_before)
      continue;
    // No we could say this location will change the liberty of the group.
    ids->ids[i4] = group_id;
    ids->colors[i4] = board->_groups[group_id].color;
    ids->group_liberties[i4] = board->_groups[group_id].liberties;
  }
  ENDFOR4
}

static inline bool isSuicideMove(const GroupId4* ids) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // Prevent any suicide moves.
  if (ids->liberty > 0)
    return false;

  int cnt_our_group_liberty_more_than_1 = 0;
  int cnt_enemy_group_liberty_1 = 0;
  for (int i = 0; i < 4; ++i) {
    if (ids->ids[i] == 0)
      continue;
    if (ids->colors[i] == ids->player) {
      if (ids->group_liberties[i] > 1)
        cnt_our_group_liberty_more_than_1++;
    } else {
      if (ids->group_liberties[i] == 1)
        cnt_enemy_group_liberty_1++;
    }
  }

  // If the following conditions holds, then it is a suicide move:
  // 1. There is no friendly group (or all friendly groups has only one
  // liberty),
  // 2. Our own liberty is zero.
  // 3. All enemy liberties are great than 1, which means we cannot kill any
  // enemy groups.
  if (cnt_our_group_liberty_more_than_1 == 0 && cnt_enemy_group_liberty_1 == 0)
    return true;

  return false;
}

static inline bool isSimpleKoViolation(const Board* b, Coord c, Stone player) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  if (b->_simple_ko == c && b->_ko_age == 0 && b->_simple_ko_color == player) {
    // printf("Ko violations!!  (%d, %d), player = %d\n", X(c), Y(c), player);
    return true;
  } else
    return false;
}

#define MAX_LADDER_SEARCH 1024
int checkLadderUseSearch(Board* board, Stone victim, int* num_call, int depth) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  (*num_call)++;
  Coord c = board->_last_move;
  Coord c2 = board->_last_move2;
  unsigned short id = board->_infos[c].id;
  unsigned short lib = board->_groups[id].liberties;
  // char buf[30];
  GroupId4 ids;

  if (victim == OPPONENT(board->_next_player)) {
    // Capturer to play. He can choose two ways to capture.

    // Captured.
    if (lib == 1)
      return depth;
    // Not able to capture.
    if (lib >= 3)
      return 0;
    // Check if c's vicinity's two empty locations.
    Coord escape[2];
    int num_escape = 0;
    FOR4(c, _, cc) {
      if (board->_infos[cc].color == S_EMPTY) {
        escape[num_escape++] = cc;
      }
    }
    ENDFOR4
    // Not a ladder.
    if (num_escape <= 1)
      return 0;

    // Play each possibility and recurse.
    // We can avoid copying if we are sure one branch cannot be right and only
    // trace down the other.
    int freedom[2];
    Coord must_block = M_PASS;
    for (int i = 0; i < 2; ++i) {
      freedom[i] = 0;
      FOR4(escape[i], _, cc) {
        if (board->_infos[cc].color == S_EMPTY) {
          freedom[i]++;
        }
      }
      ENDFOR4
      if (freedom[i] == 3) {
        // Then we have to block this.
        must_block = escape[i];
        break;
      }
    }

    // Check if we have too many branches. If so, stopping the branching.
    if (must_block == M_PASS && *num_call >= MAX_LADDER_SEARCH) {
      must_block = escape[0];
    }

    if (must_block != M_PASS) {
      // It suffices to only play must_block.
      if (TryPlay2(board, must_block, &ids)) {
        Play(board, &ids);
        int final_depth =
            checkLadderUseSearch(board, victim, num_call, depth + 1);
        if (final_depth > 0)
          return final_depth;
      }
    } else {
      // printf("isLadderUseSearch: Branching:\n");
      // printf("Choice 1: %s\n", get_move_str(escape[0], board->_next_player,
      // buf));
      // printf("Choice 2: %s\n", get_move_str(escape[1], board->_next_player,
      // buf));
      // showBoard(board, SHOW_ALL);

      // We need to play both. This should seldomly happen.
      Board b_next;
      copyBoard(&b_next, board);
      if (TryPlay2(&b_next, escape[0], &ids)) {
        Play(&b_next, &ids);
        int final_depth =
            checkLadderUseSearch(&b_next, victim, num_call, depth + 1);
        if (final_depth > 0)
          return final_depth;
      }

      if (TryPlay2(board, escape[1], &ids)) {
        Play(board, &ids);
        int final_depth =
            checkLadderUseSearch(board, victim, num_call, depth + 1);
        if (final_depth > 0)
          return final_depth;
      }
    }
  } else {
    // Victim to play. In general he only has one choice because he is always in
    // atari.
    // If the capturer place a stone in atari, then the capture fails.
    if (lib == 1)
      return 0;
    // Otherwise the victim need to continue fleeing.
    Coord flee_loc = M_PASS;
    FOR4(c2, _, cc) {
      if (board->_infos[cc].color == S_EMPTY) {
        flee_loc = cc;
        break;
      }
    }
    ENDFOR4
    // Make sure flee point is not empty
    if (flee_loc == M_PASS) {
      showBoard(board, SHOW_ALL);
      error("Error!! isLadderUseSearch is wrong!\n");
      return 0;
    }
    if (TryPlay2(board, flee_loc, &ids)) {
      Play(board, &ids);
      unsigned char id = board->_infos[flee_loc].id;
      if (board->_groups[id].liberties >= 3)
        return 0;
      if (board->_groups[id].liberties == 2) {
        // Check if the neighboring enemy stone has only one liberty, if so,
        // then it is not a ladder.
        FOR4(flee_loc, _, cc) {
          if (board->_infos[cc].color != OPPONENT(victim))
            continue;
          unsigned char id2 = board->_infos[cc].id;
          // If the enemy group is in atari but our group has 2 liberties, then
          // it is not a ladder.
          if (board->_groups[id2].liberties == 1)
            return 0;
        }
        ENDFOR4
      }
      int final_depth =
          checkLadderUseSearch(board, victim, num_call, depth + 1);
      if (final_depth > 0)
        return final_depth;
    }
  }
  return 0;
}


Coord getSimpleKoLocation(const Board* board, Stone* player) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  if (board->_ko_age == 0 && board->_simple_ko != M_PASS) {
    if (player != nullptr)
      *player = board->_simple_ko_color;
    return board->_simple_ko;
  } else {
    return M_PASS;
  }
}



void RemoveStoneAndAddLiberty(Board* board, Coord c) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // First perform an analysis.
  GroupId4 ids;
  StoneLibertyAnalysis(board, board->_next_player, c, &ids);

  // Check nearby groups and add their liberties. Note that we need to skip our
  // own group (since it will be removed eventually).
  for (int i = 0; i < 4; ++i) {
    unsigned short id = ids.ids[i];
    if (id == 0 || id == board->_infos[c].id)
      continue;
    board->_groups[id].liberties++;
  }

  // printf("RemoveStoneAndAddLiberty: Remove stone at (%d, %d), belonging to
  // Group %d\n", X(c), Y(c), board->_infos[c].id);
  set_color(board, c, S_EMPTY);
  board->_infos[c].id = 0;
  board->_infos[c].next = 0;
}

// Group related opreations.
bool EmptyGroup(Board* board, unsigned short group_id) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  if (group_id == 0)
    return false;
  Coord c = board->_groups[group_id].start;
  while (c != 0) {
    // printf("Remove stone (%d, %d)\n", X(c), Y(c));
    Coord next = board->_infos[c].next;
    RemoveStoneAndAddLiberty(board, c);
    c = next;
  }
  // Note this group might be visited again in RemoveAllEmptyGroups, if:
  // There are two empty groups, one with id and the other is the last group.
  // Then when we copy the last group to the former id, we might visit the last
  // group
  // again with invalid start pointer. This is bad.
  // However, if _empty_group_ids will be sorted, then it doesn't matter.
  // board->_groups[group_id].start = 0;
  board->_removed_group_ids[board->_num_group_removed++] = group_id;
  // empty_group_ids[(*next_empty_group) ++] = group_id;
  // board->_empty_group_ids[board->_next_empty_group++] = group_id;
  if (board->_num_group_removed > 4) {
    error("Error! _next_empty_group > 4!! \n");
  }
  return true;
}

void SimpleSort(unsigned char* ids, int n) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // Sort a vector in descending order.
  for (int i = 0; i < n; ++i) {
    for (int j = i + 1; j < n; ++j) {
      if (ids[i] < ids[j]) {
        unsigned short tmp = ids[i];
        ids[i] = ids[j];
        ids[j] = tmp;
      }
    }
  }
}

void RemoveAllEmptyGroups(Board* board) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // A simple sorting on the empty group id.
  SimpleSort(board->_removed_group_ids, board->_num_group_removed);

  for (int i = 0; i < board->_num_group_removed; ++i) {
    unsigned short id = board->_removed_group_ids[i];
    // printf("Remove empty group %d\n", id);
    unsigned short last_id = board->_num_groups - 1;
    if (id != last_id) {
      // Swap with the last entry.
      // Copy the structure.
      memcpy(&board->_groups[id], &board->_groups[last_id], sizeof(Group));
      TRAVERSE(board, id, c) {
        board->_infos[c].id = id;
      }
      ENDTRAVERSE
      // No need to map ids once the ids are sorted in an descending order.
      // Check the following empty group ids, if they are at the end, map their
      // id to the new position.
    }
    board->_num_groups--;
  }
  // Clear _num_group_removed when play starts.
  // board->_next_empty_group = 0;
}


unsigned short createNewGroup(Board* board, Coord c, int liberty) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  unsigned short id = board->_num_groups++;
  board->_groups[id].color = board->_infos[c].color;
  board->_groups[id].start = c;
  board->_groups[id].liberties = liberty;
  board->_groups[id].stones = 1;

  board->_infos[c].id = id;
  board->_infos[c].next = 0;
  return id;
}

// Merge a single stone into an existing group. In this case, no group
// deletion/move
// is needed.
// Here the liberty is that of the single stone (raw liberty).
bool MergeToGroup(Board* board, Coord c, unsigned short id) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // Place the stone.
  set_color(board, c, board->_groups[id].color);
  board->_infos[c].last_placed = board->_ply;

  board->_infos[c].id = id;
  // Put the new stone to the beginning of the group.
  board->_infos[c].next = board->_groups[id].start;
  board->_groups[id].start = c;
  board->_groups[id].stones++;
// We need to be careful about the liberty, since some liberties of the new
// stone may also be liberty of the group to be merged.
#define SAME_ID(c) (board->_infos[(c)].id == id)

  bool lt = !SAME_ID(GO_LT(c));
  bool lb = !SAME_ID(GO_LB(c));
  bool rt = !SAME_ID(GO_RT(c));
  bool rb = !SAME_ID(GO_RB(c));

  if (EMPTY(board->_infos[GO_L(c)].color) && lt && lb && !SAME_ID(GO_LL(c)))
    board->_groups[id].liberties++;
  if (EMPTY(board->_infos[GO_R(c)].color) && rt && rb && !SAME_ID(GO_RR(c)))
    board->_groups[id].liberties++;
  if (EMPTY(board->_infos[GO_T(c)].color) && lt && rt && !SAME_ID(GO_TT(c)))
    board->_groups[id].liberties++;
  if (EMPTY(board->_infos[GO_B(c)].color) && lb && rb && !SAME_ID(GO_BB(c)))
    board->_groups[id].liberties++;

#undef SAME_ID

  return true;
}

// Merge two groups into one.
// The resulting liberties might not be right and need to be recomputed.
unsigned short
MergeGroups(Board* board, unsigned short id1, unsigned short id2) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // printf("merge beteween %d and %d", id1, id2);
  // Same id, no merge.
  if (id1 == id2)
    return id1;

  // To save computation power, we want to traverse through the group with small
  // number of stones.
  if (board->_groups[id2].stones > board->_groups[id1].stones)
    return MergeGroups(board, id2, id1);

  // Merge
  // Find the last stone in id2.
  Coord last_c_in_id2 = 0;
  TRAVERSE(board, id2, c) {
    board->_infos[c].id = id1;
    last_c_in_id2 = c;
  }
  ENDTRAVERSE

  // Make connections. Put id2 group in front of id1.
  board->_infos[last_c_in_id2].next = board->_groups[id1].start;
  board->_groups[id1].start = board->_groups[id2].start;
  //
  // Other quantities.
  board->_groups[id1].stones += board->_groups[id2].stones;
  // Note that the summed liberties is not right (since multiple groups might
  // share liberties, therefore we need to recompute it).
  board->_groups[id1].liberties = -1;

  // Make id2 an empty group.
  board->_groups[id2].start = 0;
  board->_removed_group_ids[board->_num_group_removed++] = id2;
  // board->_empty_group_ids[board->_next_empty_group++] = id2;

  // Equivalently, we could do:
  // board->_groups[id2].start = 0;
  // EmptyGroup(board, id2);
  return id1;
}

bool RecomputeGroupLiberties(Board* board, unsigned short id) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // Put all neighboring spaces into a set, and count the number.
  // Borrowing _info.next for counting. No extra space needed.
  if (id == 0)
    return false;
  short liberty = 0;
  TRAVERSE(board, id, c){FOR4(c, _, c4){Info* info = &board->_infos[c4];
  if (G_EMPTY(info->id) && info->next == 0) {
    // printf("RecomputeGroupLiberties: liberties of Group %d at (%d, %d)",
    // id, X(c4), Y(c4));
    info->next = 1;
    liberty++;
  }
}
ENDFOR4
}
ENDTRAVERSE
// Second traverse to set all .next to be zero.
TRAVERSE(board, id, c){FOR4(c, _, c4){Info* info = &board->_infos[c4];
if (G_EMPTY(info->id))
  info->next = 0;
}
ENDFOR4
}
ENDTRAVERSE

board->_groups[id].liberties = liberty;
return true;
}

bool TryPlay2(const Board* board, Coord m, GroupId4* ids) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  return TryPlay(board, X(m), Y(m), board->_next_player, ids);
}

bool TryPlay(const Board* board, int x, int y, Stone player, GroupId4* ids) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // Place the stone on the coordinate, and update other structures.
  myassert(board, "TryPlay: Board is nil!");
  myassert(ids, "TryPlay: GroupIds4 is nil!");

  Coord c = OFFSETXY(x, y);
  if (c == M_PASS) {
    memset(ids, 0, sizeof(GroupId4));
    ids->c = c;
    ids->player = player;

    return true;
  }

  // Return false if the move is out of bound.
  if (x < 0 || x >= BOARD_SIZE || y < 0 || y >= BOARD_SIZE)
    return false;

  // printf("Move: (%d, %d), player = %d\n", X(c), Y(c), player);
  // Cannot place a move here.
  if (!EMPTY(board->_infos[c].color)) {
    // printf("Already has stone! (%d, %d) with color = %d (player color =
    // %d)\n", X(c), Y(c), board->_infos[c].color, player);
    return false;
  }

  // Prevent any simple ko violation..
  if (isSimpleKoViolation(board, c, player))
    return false;

  // Perform an analysis.
  StoneLibertyAnalysis(board, player, c, ids);
  // Prevent any suicide move.
  if (isSuicideMove(ids)) {
    // printf("Is suicide move! (%d, %d), player = %d\n", X(c), Y(c), player);
    return false;
  }

  return true;
}



static inline void update_next_move(Board* board, Coord c, Stone player) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  board->_next_player = OPPONENT(player);

  board->_last_move4 = board->_last_move3;
  board->_last_move3 = board->_last_move2;
  board->_last_move2 = board->_last_move;
  board->_last_move = c;

  // Compute the hash code with c, player and ply.
  // unsigned long seed = MOVE_HASH(c, player, board->_ply);
  // board->hash ^= fast_random64(&seed);

  board->_ply++;
}



bool Play(Board* board, const GroupId4* ids) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  myassert(board, "Play: Board is nil!");
  myassert(ids, "Play: GroupIds4 is nil!");

  board->_num_group_removed = 0;

  // Place the stone on the coordinate, and update other structures.
  Coord c = ids->c;
  Stone player = ids->player;
  if (c == M_PASS) {
    update_next_move(board, c, player);
    return isGameEnd(board);
  }

  short new_id = 0;
  unsigned short liberty = ids->liberty;
  short total_capture = 0;
  Coord capture_c = 0;
  bool merge_two_groups_called = false;
  for (int i = 0; i < 4; ++i) {
    // printf("Analysis #%d: id = %d, color = %d, liberty = %d\n", i,
    // ids->ids[i], ids->colors[i], ids->group_liberties[i]);
    // Skip nullptr group.
    if (ids->ids[i] == 0)
      continue;
    unsigned short id = ids->ids[i];
    Group* g = &board->_groups[id];

    Stone s = g->color;
    // The group adjacent to it lose one liberty.
    --g->liberties;

    if (s == player) {
      // Self-group.
      if (new_id == 0) {
        // Merge the current stone with the current group.
        MergeToGroup(board, c, id);
        new_id = id;
        // printf("Merge with group %d, preducing id = %d", id, new_id);
      } else {
        // int prev_new_id = new_id;
        // Merge two large groups.
        new_id = MergeGroups(board, new_id, id);
        merge_two_groups_called = true;
        // printf("Merge with group %d with existing id %d, producing id = %d",
        // id, prev_new_id, new_id);
      }
    } else {
      // Enemy group, If the enemy group has zero liberties, it is killed.
      if (g->liberties == 0) {
        // printf("kill group %d of size %d\n", id, g->stones);
        if (player == S_BLACK)
          board->_b_cap += g->stones;
        else
          board->_w_cap += g->stones;
        total_capture += g->stones;

        // Compute the adjacent enemy point.
        capture_c = c + delta4[i];

        // Add our liberties if the new stone is not yet forming a group.
        // Otherwise the liberties of a dead group's surrounding groups will be
        // taken care of automatically.
        if (new_id == 0) {
          FOR4(c, _, c4) {
            if (board->_infos[c4].id == id)
              liberty++;
          }
          ENDFOR4
        }
        // Remove stones of the group.
        EmptyGroup(board, id);
      }
    }
  }
  // if (new_id > 0) RecomputeGroupLiberties(board, new_id);
  if (merge_two_groups_called)
    RecomputeGroupLiberties(board, new_id);
  if (new_id == 0) {
    // It has not merged with other groups, create a new one.
    set_color(board, c, player);
    // Place the stone.
    board->_infos[c].last_placed = board->_ply;

    new_id = createNewGroup(board, c, liberty);
  }

  // Check simple ko conditions.
  const Group* g = &board->_groups[new_id];
  if (g->liberties == 1 && g->stones == 1 && total_capture == 1) {
    board->_simple_ko = capture_c;
    board->_simple_ko_color = OPPONENT(player);
    board->_ko_age = 0;
  } else {
    board->_ko_age++;
  }

  // We need to run it in the end. After that all group index will be invalid.
  RemoveAllEmptyGroups(board);

  // Finally add the counter.
  update_next_move(board, c, player);
  return false;
}

void str_concat(char* buf, int* len, const char* str) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  *len += sprintf(buf + *len, "%s", str);
}

void showBoard2Buf(const Board* board, ShowChoice choice, char* buf) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // Warning [TODO]: possibly buffer overflow.
  char buf2[30];
  int len = 0;
  str_concat(buf, &len, "   ");
  str_concat(buf, &len, BOARD_PROMPT);
  str_concat(buf, &len, "\n");

  char stone[3];
  stone[2] = 0;
  for (int j = BOARD_SIZE - 1; j >= 0; --j) {
    len += sprintf(buf + len, "%2d ", j + 1);
    for (int i = 0; i < BOARD_SIZE; ++i) {
      Coord c = OFFSETXY(i, j);
      Stone s = board->_infos[c].color;
      if (HAS_STONE(s)) {
        if (c == board->_last_move && choice >= SHOW_LAST_MOVE) {
          if (s == S_BLACK)
            strcpy(stone, "X)");
          else
            strcpy(stone, "O)");
        } else {
          if (s == S_BLACK)
            strcpy(stone, "X ");
          else
            strcpy(stone, "O ");
        }
      } else if (s == S_EMPTY) {
        if (STAR_ON(i, j))
          strcpy(stone, "+ ");
        else
          strcpy(stone, ". ");
      } else
        strcpy(stone, "# ");
      str_concat(buf, &len, stone);
    }
    len += sprintf(buf + len, "%d", j + 1);
    if (j == BOARD_SIZE / 2 + 1) {
      len += sprintf(
          buf + len, "     WHITE (O) has captured %d stones", board->_w_cap);
    } else if (j == BOARD_SIZE / 2) {
      len += sprintf(
          buf + len, "     BLACK (X) has captured %d stones", board->_b_cap);
    }
    str_concat(buf, &len, "\n");
  }
  str_concat(buf, &len, "   ");
  str_concat(buf, &len, BOARD_PROMPT);
  if (choice == SHOW_ALL) {
    len += sprintf(buf + len, "\n   #Groups = %d", board->_num_groups - 1);
    len += sprintf(buf + len, "\n   #ply = %d", board->_ply);
    len += sprintf(
        buf + len,
        "\n   Last move = %s",
        get_move_str(board->_last_move, OPPONENT(board->_next_player), buf2));
    len += sprintf(
        buf + len,
        "\n   Last move2 = %s",
        get_move_str(board->_last_move2, board->_next_player, buf2));
    len += sprintf(
        buf + len,
        "\n   Ko point = %s [Age = %d]",
        get_move_str(board->_simple_ko, board->_simple_ko_color, buf2),
        board->_ko_age);
  }
}

void showBoard(const Board* board, ShowChoice choice) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  // Simple function to show board.
  char buf[2000];
  showBoard2Buf(board, choice, buf);
  // Finally print
  fprintf(stderr, "%s", buf);
}




bool isGameEnd(const Board* board) {
  display_debug_info("", __FUNCTION__, BLUE_B);

  return board->_ply > 1 &&
      ((board->_last_move == M_PASS && board->_last_move2 == M_PASS));
}

// Utilities..Here I assume buf has sufficient space (e.g., >= 30).
char* get_move_str(Coord m, Stone player, char* buf) {
  display_debug_info("", __FUNCTION__, BLUE_B);
  
  const char cols[] = "ABCDEFGHJKLMNOPQRST";
  char p = '?';
  switch (player) {
    case S_WHITE:
      p = 'W';
      break;
    case S_BLACK:
      p = 'B';
      break;
    case S_EMPTY:
      p = ' ';
      break;
    case S_OFF_BOARD:
      p = '#';
      break;
  }
  if (m == M_PASS) {
    sprintf(buf, "%c PASS", p);
  } else if (m == M_INVALID) {
    sprintf(buf, "%c INVALID", p);
  } else {
    sprintf(buf, "%c %c%d", p, cols[X(m)], Y(m) + 1);
  }
  return buf;
}

















