#!/usr/bin/env python

import sys
import inspect
import numpy as np
import json
from time import sleep

sys.path.append('..')
from py.CheckersMoves import get_all_moves

moves_for_human = get_all_moves()

def is_even(num):
    return num % 2 == 0

def is_game_piece(y, x):
    return (is_even(y) and (not is_even(x))) or ((not is_even(y)) and is_even(x))  

def getValidMoves(batch, game_idx):
  moves = {}
  valid_moves_binary = batch.GC.getGame(game_idx).getValidMoves()
  for idx in range(len(valid_moves_binary)):
    if valid_moves_binary[idx]:
      i = "{0:036b}".format(moves_for_human[idx][0])[::-1]
      index = [pos for pos, char in enumerate(i) if char == "1"]
      i1, i2 = index
      buff1 = (1 + i1 - i1 // 9) - 1
      buff2 = (1 + i2 - i2 // 9) - 1
      x1, y1 = (6 - (buff1) % 4 * 2 + ((buff1) // 4) % 2, 7 - (buff1) // 4)
      x2, y2 = (6 - (buff2) % 4 * 2 + ((buff2) // 4) % 2, 7 - (buff2) // 4)
      if not moves_for_human[idx][1]:
        x1, y1, x2, y2 = x2, y2, x1, y1
      str_move_from = "y_" + str(y1) + " x_" + str(x1)
      str_move_to = "y_" + str(y2) + " x_" + str(x2)

      moves[idx] = [str_move_from, str_move_to]
  return moves


def boardToJsonCheckers(curr_board, valid_moves_human, current_player, user_id):
  # valid_moves = board.get_legal_moves(current_player)

  result = {}

  result["user_id"] = user_id
  result["rotate"] = current_player

  result["valid_moves"] = {}
  for key in valid_moves_human:
    move = valid_moves_human[key]
    str_move_from = move[0]
    str_move_to = move[1]
    if str_move_from not in result["valid_moves"]:
      result["valid_moves"][str_move_from] = []
    result["valid_moves"][str_move_from].append(str_move_to)
  
  for y, row in enumerate(curr_board):
    res_row = {}
    for x, piece in enumerate(row):
      if piece == 1:
        res_row[x] = "piece white"
      elif piece == 3:
        res_row[x] = "piece white king"
      elif piece == -1:
        res_row[x] = "piece black"
      elif piece == -3:
        res_row[x] = "piece black king"
      elif is_game_piece(y, x):
        res_row[x] = "piece empty"
      else:
        res_row[x] = "not_game"

      str_move = "y_" + str(y) + " x_" + str(x)
      if str_move in result["valid_moves"]:
        res_row[x] += " can_move"

    result[y] = res_row

  # print("result ", result)
  return result




