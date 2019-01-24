#include "CheckersFeature.h"
#include "CheckersState.h"

static float* board_plane(float* features, int idx) {
  // возвращаем ячейку памяти с которой нужно заполнять наш массив
  return features + idx * CHECKERS_BOARD_SIZE * CHECKERS_BOARD_SIZE;
}

#define LAYER(idx) board_plane(features, idx)


void CheckersFeature::getKings(int player, float* data) const {
  std::array<std::array<int, 8>, 8>   observation;
  observation = get_observation(s_.board(), player);
  
  for (int y = 0; y < CHECKERS_BOARD_SIZE; ++y) {
    for (int x = 0; x < CHECKERS_BOARD_SIZE; ++x) {
      if (observation[y][x] == 3)
        data[y * CHECKERS_BOARD_SIZE + x] = 1;
    }
  }
}


void CheckersFeature::getPawns(int player, float* data) const {
  std::array<std::array<int, 8>, 8>   observation;
  observation = get_observation(s_.board(), player);
  
  for (int y = 0; y < CHECKERS_BOARD_SIZE; ++y) {
    for (int x = 0; x < CHECKERS_BOARD_SIZE; ++x) {
      if (observation[y][x] == 1)
        data[y * CHECKERS_BOARD_SIZE + x] = 1;
    }
  }
}

// // If player == S_EMPTY, get history of both sides.
// bool CheckersFeature::getHistory(int player, float* data) const {
//   // const Board* _board = &s_.board();

//   // memset(data, 0, kBoardRegion * sizeof(float));
//   // for (int i = 0; i < BOARD_SIZE; ++i) {
//   //   for (int j = 0; j < BOARD_SIZE; ++j) {
//   //     Coord c = OFFSETXY(i, j);
//   //     if (S_ISA(_board->_infos[c].color, player))
//   //       data[transform(i, j)] = _board->_infos[c].last_placed;
//   //   }
//   // }
//   return true;
// }

// bool CheckersFeature::getHistoryExp(int player, float* data) const {
//   // const Board* _board = &s_.board();

//   // memset(data, 0, kBoardRegion * sizeof(float));
//   // for (int i = 0; i < BOARD_SIZE; ++i) {
//   //   for (int j = 0; j < BOARD_SIZE; ++j) {
//   //     Coord c = OFFSETXY(i, j);
//   //     if (S_ISA(_board->_infos[c].color, player)) {
//   //       data[transform(i, j)] =
//   //           exp((_board->_infos[c].last_placed - _board->_ply) / 10.0);
//   //     }
//   //   }
//   // }
//   return true;
// }


// Получаем от питона выделенную память из GCWrapper.run() smem
// (зависит от количества фич и размера доски)
// и заполняем ее нашими признаками
// если фич несколько(на нашем примере их 6)
// то получаем массив вида 6x91, который переводим в одномерный массив размером 546
void CheckersFeature::extract(std::vector<float>* features) const {
  features->resize(CHECKERS_NUM_FEATURES * kBoardRegion);
  extract(&(*features)[0]);
}

void CheckersFeature::extract(float* features) const {
  const CheckersBoard* _board = &s_.board();
  // берем активного игрока
  int active_player = _board->active;
  int passive_player = _board->passive;

  std::fill(features, features + CHECKERS_NUM_FEATURES * kBoardRegion, 0.0);
  // Save the current board state to game state.
  // заполнаем слои нашего бинарого признака для передачи 
  // в python по ключу "checkers_s"
  // не забываем задефайнить эти ключи в game_feature.h
  // фишки наши, фишки врага
  getPawns(active_player, LAYER(0));
  getKings(active_player, LAYER(1));
  getPawns(passive_player, LAYER(2));
  getKings(passive_player, LAYER(3));

  // кто сейчас ходит
  float* black_indicator = LAYER(4);
  float* white_indicator = LAYER(5);
  if (active_player == BLACK_PLAYER)
    std::fill(black_indicator, black_indicator + kBoardRegion, 1.0);
  else
    std::fill(white_indicator, white_indicator + kBoardRegion, 1.0);
}
