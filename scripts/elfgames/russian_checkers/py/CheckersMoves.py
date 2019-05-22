#!/usr/bin/env python

""" 
  Way to collect all possible moves for our board,
  becouse the board is represented by six numbers
  and each turn represent by a number. 
  Also take a look at HashAllMoves.h
"""
def get_all_moves_russian_checkers():
  UNUSED_BITS = 0b100000000100000000100000000100000000
  
  # code of jump
  #      0b100000000100000000100000000100000000
  rf_0 = 0b100001111111101111111101111111101111 ^ UNUSED_BITS
  rf_1 = 0b100000000111101110111101110111101110 ^ UNUSED_BITS
  rf_2 = 0b100000000100001110111001110111001110 ^ UNUSED_BITS
  rf_3 = 0b100000000100000000111001100111001100 ^ UNUSED_BITS
  rf_4 = 0b100000000100000000100001100110001100 ^ UNUSED_BITS
  rf_5 = 0b100000000100000000100000000110001000 ^ UNUSED_BITS
  rf_6 = 0b100000000100000000100000000100001000 ^ UNUSED_BITS
  #            0b000100010001000100010001000100010001
  rf_cod_0  = [0b000000000000000000000000000000010001 << i for (i, bit) in enumerate(bin(rf_0)[::-1]) if bit == '1']
  rf_cod_1  = [0b000000000000000000000000000100000001 << i for (i, bit) in enumerate(bin(rf_1)[::-1]) if bit == '1']
  rf_cod_2  = [0b000000000000000000000001000000000001 << i for (i, bit) in enumerate(bin(rf_2)[::-1]) if bit == '1']
  rf_cod_3  = [0b000000000000000000010000000000000001 << i for (i, bit) in enumerate(bin(rf_3)[::-1]) if bit == '1']
  rf_cod_4  = [0b000000000000000100000000000000000001 << i for (i, bit) in enumerate(bin(rf_4)[::-1]) if bit == '1']
  rf_cod_5  = [0b000000000001000000000000000000000001 << i for (i, bit) in enumerate(bin(rf_5)[::-1]) if bit == '1']
  rf_cod_6  = [0b000000010000000000000000000000000001 << i for (i, bit) in enumerate(bin(rf_6)[::-1]) if bit == '1']

  #      0b100000000100000000100000000100000000
  lf_0 = 0b100000111111110111111110111111110111 ^ UNUSED_BITS
  lf_1 = 0b100000000101110111101110111101110111 ^ UNUSED_BITS
  lf_2 = 0b100000000100000011101110011101110011 ^ UNUSED_BITS
  lf_3 = 0b100000000100000000100110011100110011 ^ UNUSED_BITS
  lf_4 = 0b100000000100000000100000001100110001 ^ UNUSED_BITS
  lf_5 = 0b100000000100000000100000000100010001 ^ UNUSED_BITS
  #           0b000001000010000100001000010000100001
  lf_cod_0 = [0b000000000000000000000000000000100001 << i for (i, bit) in enumerate(bin(lf_0)[::-1]) if bit == '1']
  lf_cod_1 = [0b000000000000000000000000010000000001 << i for (i, bit) in enumerate(bin(lf_1)[::-1]) if bit == '1']
  lf_cod_2 = [0b000000000000000000001000000000000001 << i for (i, bit) in enumerate(bin(lf_2)[::-1]) if bit == '1']
  lf_cod_3 = [0b000000000000000100000000000000000001 << i for (i, bit) in enumerate(bin(lf_3)[::-1]) if bit == '1']
  lf_cod_4 = [0b000000000010000000000000000000000001 << i for (i, bit) in enumerate(bin(lf_4)[::-1]) if bit == '1']
  lf_cod_5 = [0b000001000000000000000000000000000001 << i for (i, bit) in enumerate(bin(lf_5)[::-1]) if bit == '1']

  rb = rf_cod_0 + rf_cod_1 + rf_cod_2 + rf_cod_3 + rf_cod_4 + rf_cod_5 + rf_cod_6
  lb = lf_cod_0 + lf_cod_1 + lf_cod_2 + lf_cod_3 + lf_cod_4 + lf_cod_5
  rf = rf_cod_0 + rf_cod_1 + rf_cod_2 + rf_cod_3 + rf_cod_4 + rf_cod_5 + rf_cod_6
  lf = lf_cod_0 + lf_cod_1 + lf_cod_2 + lf_cod_3 + lf_cod_4 + lf_cod_5

  total_moves_cod = rf + lf + rb + lb

  all_moves = []

  for move in total_moves_cod:
    if (move, True) in all_moves:
      all_moves.append((move, False))
    else:
      all_moves.append((move, True))

  return all_moves