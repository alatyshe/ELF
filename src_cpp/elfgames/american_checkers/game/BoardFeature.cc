#include "BoardFeature.h"
#include "GameState.h"

static float* board_plane(float* features, int idx) {
  return features + idx * CHECKERS_BOARD_SIZE * CHECKERS_BOARD_SIZE;
}

// features param will taken from parent function 
#define LAYER(idx) board_plane(features, idx)

void BoardFeature::getKings(GameBoard board, int player, float* data) const {
  std::array<std::array<int, 8>, 8> observation;
  observation = GetObservation(board, player);
  
  for (int y = 0; y < CHECKERS_BOARD_SIZE; ++y) {
    for (int x = 0; x < CHECKERS_BOARD_SIZE; ++x) {
      if (observation[y][x] == 3)
        data[y * CHECKERS_BOARD_SIZE + x] = 1;
    }
  }
}

void BoardFeature::getPawns(GameBoard board, int player, float* data) const {
  std::array<std::array<int, 8>, 8> observation;
  observation = GetObservation(board, player);
  
  for (int y = 0; y < CHECKERS_BOARD_SIZE; ++y) {
    for (int x = 0; x < CHECKERS_BOARD_SIZE; ++x) {
      if (observation[y][x] == 1)
        data[y * CHECKERS_BOARD_SIZE + x] = 1;
    }
  }
}

// Extract game state, this method calls from GameFeature::extractState()
// Filling the memory for submission to the assessment in the neural network.
// vector features - depends on the number of features and size of the board.
// For example we have 6 CHECKERS_NUM_FEATURES defined in GameBoard.h
// and board size 8 x 8 = 64, so we have 2 dim array 6 x 64
// wich transform into 1d array 384 lenght.
// From python side we get this information by the key "s".
void BoardFeature::extract(std::vector<float>* features) const {
  features->resize(CHECKERS_NUM_FEATURES * kBoardRegion);
  extract(&(*features)[0]);
}

void BoardFeature::extract(float* features) const {
  int active_player;
  int passive_player;
  GameBoard _board;

  std::fill(features, features + CHECKERS_NUM_FEATURES * kBoardRegion, 0.0);
  int history_size = s_.getHistory().size();
  int i = 0;
  for (int j = history_size; j < MAX_CHECKERS_HISTORY; j++)
    i++;

  for (int k = 0; i < MAX_CHECKERS_HISTORY; i++, k++) {
    _board = s_.getHistory()[k];

    active_player = _board.active;
    passive_player = _board.passive;

    getPawns(_board, active_player, LAYER(6 * i + 0));
    getKings(_board, active_player, LAYER(6 * i + 1));
    getPawns(_board, passive_player, LAYER(6 * i + 2));
    getKings(_board, passive_player, LAYER(6 * i + 3));
    // the player on move
    float* black_indicator = LAYER(6 * i + 4);
    float* white_indicator = LAYER(6 * i + 5);
    if (active_player == BLACK_PLAYER)
      std::fill(black_indicator, black_indicator + kBoardRegion, 1.0);
    else
      std::fill(white_indicator, white_indicator + kBoardRegion, 1.0);

  }
}
