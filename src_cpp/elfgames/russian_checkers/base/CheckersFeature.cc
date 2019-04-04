#include "CheckersFeature.h"
#include "CheckersState.h"

static float* board_plane(float* features, int idx) {
  return features + idx * CHECKERS_BOARD_SIZE * CHECKERS_BOARD_SIZE;
}

// features param will taken from parent function 
#define LAYER(idx) board_plane(features, idx)

void CheckersFeature::getKings(int player, float* data) const {
  std::array<std::array<int, 8>, 8> observation;
  observation = GetObservation(s_.board(), player);
  
  for (int y = 0; y < CHECKERS_BOARD_SIZE; ++y) {
    for (int x = 0; x < CHECKERS_BOARD_SIZE; ++x) {
      if (observation[y][x] == 3)
        data[y * CHECKERS_BOARD_SIZE + x] = 1;
    }
  }
}

void CheckersFeature::getPawns(int player, float* data) const {
  std::array<std::array<int, 8>, 8> observation;
  observation = GetObservation(s_.board(), player);
  
  for (int y = 0; y < CHECKERS_BOARD_SIZE; ++y) {
    for (int x = 0; x < CHECKERS_BOARD_SIZE; ++x) {
      if (observation[y][x] == 1)
        data[y * CHECKERS_BOARD_SIZE + x] = 1;
    }
  }
}

// void CheckersFeature::getHistory(int player, float* data) const {
//   const Board* _board = &s_.board();

//   for (int i = 0; i < CHECKERS_BOARD_SIZE; ++i) {
//     for (int j = 0; j < CHECKERS_BOARD_SIZE; ++j) {
//     }
//   }
//   return true;
// }

// Extract game state, this method calls from GameFeature::extractState()
// Filling the memory for submission to the assessment in the neural network.
// vector features - depends on the number of features and size of the board.
// For example we have 6 CHECKERS_NUM_FEATURES defined in checkersBoard.h
// and board size 8 x 8 = 64, so we have 2 dim array 6 x 64
// wich transform into 1d array 384 lenght.
// From python side we get this information by the key "checkers_s".
void CheckersFeature::extract(std::vector<float>* features) const {
  features->resize(CHECKERS_NUM_FEATURES * kBoardRegion);
  extract(&(*features)[0]);
}

void CheckersFeature::extract(float* features) const {
  const CheckersBoard* _board = &s_.board();
  int passive_player, active_player;

  active_player = _board->current_player;
  if (active_player == WHITE_PLAYER)
    passive_player = BLACK_PLAYER;
  else 
    passive_player = WHITE_PLAYER;

  std::fill(features, features + CHECKERS_NUM_FEATURES * kBoardRegion, 0.0);

  // Save the current board state to game state.
  getPawns(active_player, LAYER(0));
  getKings(active_player, LAYER(1));
  getPawns(passive_player, LAYER(2));
  getKings(passive_player, LAYER(3));

  // the player on move
  float* black_indicator = LAYER(4);
  float* white_indicator = LAYER(5);
  if (active_player == BLACK_PLAYER)
    std::fill(black_indicator, black_indicator + kBoardRegion, 1.0);
  else
    std::fill(white_indicator, white_indicator + kBoardRegion, 1.0);
}
