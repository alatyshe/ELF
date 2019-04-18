#!/usr/bin/env python

""" 
  Way to collect all possible moves for our board,
  becouse the board is represented by six numbers
  and each turn represent by a number. 
  Also take a look at HashAllMoves.h
"""
def get_all_moves():
  all_moves = []
  i = 0

  for y in range(8):
    for x in range(8):
      if (x + 1 < 8):
        all_moves.append([y * 8 + x, y * 8 + x + 1])
      if (x - 1 >= 0):
        all_moves.append([y * 8 + x, y * 8 + x - 1])
      if (y + 1 < 8):
        all_moves.append([y * 8 + x, (y + 1) * 8 + x])
      if (y - 1 >= 0):
        all_moves.append([y * 8 + x, (y - 1) * 8 + x])

      if (x + 2 < 8):
        all_moves.append([y * 8 + x, y * 8 + x + 2])
      if (x - 2 >= 0):
        all_moves.append([y * 8 + x, y * 8 + x - 2])
      if (y + 2 < 8):
        all_moves.append([y * 8 + x, (y + 2) * 8 + x])
      if (y - 2 >= 0):
        all_moves.append([y * 8 + x, (y - 2) * 8 + x])

  all_moves.append([-1, -1])

  return all_moves
