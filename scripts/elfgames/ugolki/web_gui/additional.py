#!/usr/bin/env python

import sys
import inspect
import numpy as np
import json
from time import sleep

sys.path.append('..')
from py.UgolkiMoves import get_all_moves_ugolki


def boardToJson(curr_board, valid_moves_human, current_player):
  # valid_moves = board.get_legal_moves(current_player)

  result = {}

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
      elif piece == -1:
        res_row[x] = "piece black"
      else:
        res_row[x] = "piece empty"

      str_move = "y_" + str(y) + " x_" + str(x)
      if str_move in result["valid_moves"]:
        res_row[x] += " can_move"

    result[y] = res_row

  # print("result ", result)
  return result




