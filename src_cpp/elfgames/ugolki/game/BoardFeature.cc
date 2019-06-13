#include "BoardFeature.h"
#include "GameState.h"

static float* board_plane(float* features, int idx) {
  return features + idx * BOARD_SIZE * BOARD_SIZE;
}

// features param will taken from parent function 
#define LAYER(idx) board_plane(features, idx)

void BoardFeature::getPawns(int player, float* data) const {
  std::array<std::array<int, 8>, 8> observation;
  observation = GetObservation(s_.board(), player);
  
  for (int y = 0; y < BOARD_SIZE; ++y) {
    for (int x = 0; x < BOARD_SIZE; ++x) {
      if (observation[y][x] == 1)
        data[y * BOARD_SIZE + x] = 1;
    }
  }
}

// void BoardFeature::getHistory(int player, float* data) const {
//   const Board* _board = &s_.board();

//   for (int i = 0; i < BOARD_SIZE; ++i) {
//     for (int j = 0; j < BOARD_SIZE; ++j) {
//     }
//   }
//   return true;
// }

// Extract game state, this method calls from GameFeature::extractState()
// Filling the memory for submission to the assessment in the neural network.
// vector features - depends on the number of features and size of the board.
// For example we have 6 NUM_FEATURES defined in GameBoard.h
// and board size 8 x 8 = 64, so we have 2 dim array 6 x 64
// wich transform into 1d array 384 lenght.
// From python side we get this information by the key "s".
void BoardFeature::extract(std::vector<float>* features) const {
  features->resize(NUM_FEATURES * kBoardRegion);
  extract(&(*features)[0]);
}

void BoardFeature::extract(float* features) const {
  const GameBoard* _board = &s_.board();

  int active_player = _board->active;
  int passive_player = (active_player == BLACK_PLAYER) ? WHITE_PLAYER : BLACK_PLAYER;

  std::fill(features, features + NUM_FEATURES * kBoardRegion, 0.0);

  // Save the current board state to game state.
  getPawns(active_player, LAYER(0));
  getPawns(passive_player, LAYER(1));

  float* black_indicator = LAYER(2);
  float* white_indicator = LAYER(3);
  if (active_player == BLACK_PLAYER)
    std::fill(black_indicator, black_indicator + kBoardRegion, 1.0);
  else
    std::fill(white_indicator, white_indicator + kBoardRegion, 1.0);
}
