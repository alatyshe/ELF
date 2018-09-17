/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <deque>
#include <queue>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "elfgames/checkers/sgf/sgf.h"

#include "board.h"
#include "board_feature.h"

// class HandicapTable {
//  private:
//   // handicap table.
//   std::unordered_map<int, std::vector<Coord>> _handicaps;

//  public:
//   HandicapTable();
//   void apply(int handi, Board* board) const;
// };







class GoState {
 public:
  GoState() {
    reset();
  }


  GoState(const GoState& s)
      : _history(s._history),
        _moves(s._moves),
        _final_value(s._final_value),
        _has_final_value(s._has_final_value) {
    copyBoard(&_board, &s._board);
  }


  void reset() {
    clearBoard(&_board);
    _moves.clear();
    _history.clear();
    _final_value = 0.0;
    _has_final_value = false;
  }

  // получаем action и делаем шаг вперед проверяя на валидность
  bool forward(const Coord& c){
    // if (c == M_INVALID) {
    //   throw std::range_error("GoState::forward(): move is M_INVALID");
    // }
    // если игра окончена = выходим
    if (terminated())
      return false;


    // GroupId4 ids;
    // if (!TryPlay2(&_board, c, &ids))
    //   return false;

    // Play(&_board, &ids);

// шаги после данного шага
    // _moves.push_back(c);
    // _history.emplace_back(_board);
    // if (_history.size() > MAX_NUM_AGZ_HISTORY)
    //   _history.pop_front();
    return true;
  }


  // проверяем валидность хода?
  bool checkMove(const Coord& c) const {
    // GroupId4 ids;
    // if (c == M_INVALID)
    //   return false;
    // return TryPlay2(&_board, c, &ids);
    return true;
  }

  // устанавливаем финальные awards
  void setFinalValue(float final_value) {
    _final_value = final_value;
    _has_final_value = true;
  }


  // финальная оценка игры
  float getFinalValue() const {
    return _final_value;
  }
  
  // закончена ли игра true/false (в плане есть ли финальная оценка)
  bool HasFinalValue() const {
    return _has_final_value;
  }

  // просто борда??
  const Board& board() const {
    return _board;
  }

  // Note that ply started from 1.
  bool justStarted() const {
    return _board._ply == 1;
  }

  // номер шага
  int getPly() const {
    return _board._ply;
  }

  // сдались ли 2-е игроков
  bool isTwoPass() const {
    // return _board._last_move == M_PASS && _board._last_move2 == M_PASS;
    return false;
  }

  // окончена ли игра
  bool terminated() const {
    // return isTwoPass() || getPly() >= BOARD_MAX_MOVE || _check_superko();
    return false;
  }

  // Последний шаг первого игрока
  Coord lastMove() const {
    return _board._last_move;
  }

  // Последний шаг второго игрока
  Coord lastMove2() const {
    return _board._last_move2;
  }

  // следующий игрок
  Stone nextPlayer() const {
    return _board._next_player;
  }



  bool moves_since(size_t* next_move_number, std::vector<Coord>* moves) const {
    // шаги после данного шага
    if (*next_move_number > _moves.size()) {
      // The move number is not right.
      return false;
    }
    moves->clear();
    for (size_t i = *next_move_number; i < _moves.size(); ++i) {
      moves->push_back(_moves[i]);
    }
    *next_move_number = _moves.size();
    return true;
  }




  const std::vector<Coord>& getAllMoves() const {
    // MY
    for (int i = 0; i < (int)_moves.size(); i++)
      std::cout << _moves.at(i) << std::endl;
    // MY END
    
    return _moves;
  }




  std::string getAllMovesString() const {
    std::stringstream ss;
    for (const Coord& c : _moves) {
      ss << "[" << coord2str2(c) << "] ";
    }
    return ss.str();
  }




  // отображает нашу борду в консоль
  std::string showBoard() const {
    char buf[2000];
    showBoard2Buf(&_board, SHOW_LAST_MOVE, buf);
    return std::string(buf) + "\n" + "Last move: " + coord2str2(lastMove()) +
        ", nextPlayer: " + (nextPlayer() == S_BLACK ? "Black" : "White") + "\n";
  }




  // оценка для текущего игрока???
  float evaluate(float komi, std::ostream* oo = nullptr) const {
    float final_score = 0.0;
    // if (_check_superko()) {
    //   final_score = nextPlayer() == S_BLACK ? 1.0 : -1.0;
    // } else {
    //   final_score = (float)simple_tt_scoring(_board, oo) - komi;
    // }

    return final_score;
  }

  // TODO: not a good design..
  const std::deque<BoardHistory>& getHistory() const {
    return _history;
  }

 protected:
  Board _board;
  std::deque<BoardHistory> _history;

  struct _BoardRecord {
    Board::Bits bits;
  };

  std::vector<Coord> _moves;
  float _final_value = 0.0;
  bool _has_final_value = false;

};




















struct GoReply {
  const BoardFeature& bf;
  Coord c;
  std::vector<float> pi;
  float value = 0;
  // Model version.
  int64_t version = -1;

  GoReply(const BoardFeature& bf) : bf(bf), pi(BOARD_NUM_ACTION, 0.0) {}
};
