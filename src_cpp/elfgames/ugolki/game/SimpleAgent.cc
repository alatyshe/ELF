#include "SimpleAgent.h"


SimpleAgent::SimpleAgent() {
  std::array<std::array<int, 8>, 8> board = {{
  { -2, -1,  0,  3,  4,  5,  6,  7},
  { -1,  0,  1,  3,  5,  6,  7,  8},
    {0,  1,  1,  2,  6,  7,  8,  9},
    {3,  3,  2,  3,  7,  8,  9,  10},
    {6,  5,  6,  7,  8,  9,  10, 11},
    {6,  6,  7,  8,  9,  10, 11, 12},
    {6,  7,  8,  9,  10, 11, 12, 13},
    {7,  8,  9,  10, 11, 12, 13, 14}
  }};
  go_destination = board;

  srand(time(0));
}


int SimpleAgent::getBoardValue(GameBoard b) {
  int res = 0;

  std::array<std::array<int, 8>, 8> board;
  board = GetObservation(b, b.active);
  for (int y = 0; y < BOARD_SIZE; y++) {
    for (int x = 0; x < BOARD_SIZE; x++) {
      if (board[y][x] == 1) {
        res += go_destination[y][x];
      }
    }
  }
  return res;
}


int SimpleAgent::GetBestMove(GameBoard board) {
  uint64_t  action;
  int       best_move = -1;
  int       best_value = std::numeric_limits<int>::max();

  std::array<int, TOTAL_NUM_ACTIONS> valid_moves = GetValidMovesBinary(board);
  for (int move = 0; move < TOTAL_NUM_ACTIONS; move++) {
    if (valid_moves[move] != 0) {

      int tmp_board_value = std::numeric_limits<int>::max();
      if (move != 416) {
        action = moves::i_to_m.find(move)->second[0];
        board.pieces[board.active] ^= action;
        tmp_board_value = getBoardValue(board);
        board.pieces[board.active] ^= action;
      } else {
        tmp_board_value = getBoardValue(board);
      }
      if (tmp_board_value < best_value && (rand() % 2 || best_move == -1)) {
        best_value = tmp_board_value;
        best_move = move;
      }
    }
  }
  return best_move;
}
