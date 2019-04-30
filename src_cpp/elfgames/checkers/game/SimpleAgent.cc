#include "SimpleAgent.h"


SimpleAgent::SimpleAgent() {
  std::array<std::array<int, 8>, 8> board_0 = {{
  { -2, -1,  0,  3,  4,  5,  6,  7},
  { -1,  0,  1,  3,  5,  6,  7,  8},
    {0,  1,  1,  2,  6,  7,  8,  9},
    {3,  3,  2,  3,  7,  8,  9,  10},
    {6,  5,  6,  7,  8,  9,  10, 11},
    {6,  6,  7,  8,  9,  10, 11, 12},
    {6,  7,  8,  9,  10, 11, 12, 13},
    {7,  8,  9,  10, 11, 12, 13, 14}
  }};
  go_first_base = board_0;

  std::array<std::array<int, 8>, 8> board_1 = {{
  {  14, 13, 12, 11, 10,  9,  8,  7},
    {13, 12, 11, 10,  9,  8,  7,  6},
    {12, 11, 10,  9,  8,  7,  6,  6},
    {11, 10,  9,  8,  7,  6,  5,  6},
    {10,  9,  8,  7,  3,  2,  3,  3},
    { 9,  8,  7,  6,  2,  1,  1,  0},
    { 8,  7,  6,  5,  3,  1,  0,  -1},
    { 7,  6,  5,  4,  3,  0,  -1, -2}
  }};
  go_second_base = board_1;


  std::array<std::array<int, 8>, 8> board_2 = {{
    {7,  6,  5,  4,  3,  2,  1,  1},
    {6,  6,  5,  4,  3,  2,  1,  1},
    {5,  5,  5,  4,  3,  2,  2,  2},
    {4,  4,  4,  4,  3,  3,  3,  3},
    {3,  3,  3,  3,  4,  4,  4,  4},
    {2,  2,  2,  3,  4,  5,  5,  5},
    {1,  1,  2,  3,  4,  5,  7,  6},
    {1,  1,  2,  3,  4,  5,  6,  7}
  }};
  leave_own_base = board_2;

  srand(time(0));
}


int SimpleAgent::getBoardValue(GameBoard board, bool swap) {
  int res = 0;

  for (int y = 0; y < BOARD_SIZE; y++) {
    for (int x = 0; x < BOARD_SIZE; x++) {
      if (board.board[y][x] == board.current_player) {
        if ((board.white_must_leave_base || board.black_must_leave_base) 
            || board._ply > 200) {
          if (swap)
            res += go_first_base[y][x];
          else 
            res += go_second_base[y][x];
        } else {
          res += leave_own_base[y][x];
        }
      }
    }
  }
  return res;
}


int SimpleAgent::GetBestMove(GameBoard board, bool swap) {
  int best_move = -1;
  int best_value = std::numeric_limits<int>::max();

  std::array<int, TOTAL_NUM_ACTIONS> valid_moves = GetValidMovesBinary(board);
  for (int move = 0; move < TOTAL_NUM_ACTIONS; move++) {
    if (valid_moves[move] != 0) {

      int tmp_board_value = std::numeric_limits<int>::max();
      if (move != 416) {
        auto action = moves::i_to_m.find(move);
        int y_start = action->second[0] / 8;
        int x_start = action->second[0] % 8;
        int y_dest = action->second[1] / 8;
        int x_dest = action->second[1] % 8;

        board.board[y_dest][x_dest] = board.board[y_start][x_start];
        board.board[y_start][x_start] = 0;
        tmp_board_value = getBoardValue(board, swap);
        board.board[y_start][x_start] = board.board[y_dest][x_dest];
        board.board[y_dest][x_dest] = 0;
      } else {
        tmp_board_value = getBoardValue(board, swap);
      }
      if (tmp_board_value < best_value && (rand() % 2 || best_move == -1)) {
        best_value = tmp_board_value;
        best_move = move;
      }
    }
  }
  return best_move;
}
