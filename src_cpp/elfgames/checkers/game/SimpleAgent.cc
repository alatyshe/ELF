#include "SimpleAgent.h"


SimpleAgent::SimpleAgent() {
  std::array<std::array<int, 8>, 8> init_board = {{
    {0,  1,  2,  3,  4,  5,  6,  7},
    {1,  2,  3,  4,  5,  6,  7,  8},
    {2,  3,  4,  5,  6,  7,  8,  9},
    {3,  4,  5,  6,  7,  8,  9,  10},
    {4,  5,  6,  7,  8,  9,  10, 11},
    {5,  6,  7,  8,  9,  10, 11, 12},
    {6,  7,  8,  9,  10, 11, 12, 13},
    {7,  8,  9,  10, 11, 12, 13, 14}
  }};
  metric = init_board;
}


int SimpleAgent::getBoardValue(GameBoard board) {
  int res = 0;

  for (int y = 0; y < BOARD_SIZE; y++) {
    for (int x = 0; x < BOARD_SIZE; x++) {
      if (board.board[y][x] == board.current_player) {
        res += metric[y][x];  
      }
    }
  }
  return res;
}


int SimpleAgent::GetBestMove(GameBoard board) {
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
        tmp_board_value = getBoardValue(board);
        board.board[y_start][x_start] = board.board[y_dest][x_dest];
        board.board[y_dest][x_dest] = 0;
      } else {
        tmp_board_value = getBoardValue(board);  
      }
      if (tmp_board_value < best_value) {
        best_value = tmp_board_value;
        best_move = move;
      }
    }
  }
  return best_move;
}
