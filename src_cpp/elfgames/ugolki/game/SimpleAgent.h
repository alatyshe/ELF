#pragma once
#include <limits>
#include "GameBoard.h"
#include "HashAllMoves.h"


class SimpleAgent {
public:
  SimpleAgent();
  ~SimpleAgent() {};

  int GetBestMove(GameBoard board); 

private:
  int getBoardValue(GameBoard board);
  std::array<std::array<int, 8>, 8> go_destination;
};