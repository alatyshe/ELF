#pragma once
#include <limits>
#include "GameBoard.h"
#include "HashAllMoves.h"


class SimpleAgent {
public:
  SimpleAgent();
  ~SimpleAgent() {};

  int GetBestMove(GameBoard board, bool swap); 


private:
  int getBoardValue(GameBoard board, bool swap);
  std::array<std::array<int, 8>, 8> go_second_base;
  std::array<std::array<int, 8>, 8> go_first_base;
  std::array<std::array<int, 8>, 8> leave_own_base;
};