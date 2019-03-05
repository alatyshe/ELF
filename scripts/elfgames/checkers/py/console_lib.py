# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import inspect
import traceback
from collections import Counter

class CheckerMoves:
  """ 
    Way to collect all possible moves for our board,
    becouse the board is represented by six numbers
    and each turn represent by a number. 
    Also take a look at HashAllMoves.h
  """
  @staticmethod
  def get_all_moves():
    UNUSED_BITS = 0b100000000100000000100000000100000000

    # jumps
    # rfj - right front jump
    rfj = 0b100000000111101110111101110111101110 ^ UNUSED_BITS
    lfj = 0b100000000101110111101110111101110111 ^ UNUSED_BITS
    rbj = 0b101110111101110111101110111100000000 ^ UNUSED_BITS
    lbj = 0b111101110111101110111101110100000000 ^ UNUSED_BITS
    # code of jump
    rfj_cod = [-0x101 << i for (i, bit) in enumerate(bin(rfj)[::-1]) if bit == '1']
    lfj_cod = [-0x401 << i for (i, bit) in enumerate(bin(lfj)[::-1]) if bit == '1']
    rbj_cod = [-0x101 << i - 8 for (i, bit) in enumerate(bin(rbj)[::-1]) if bit == '1']
    lbj_cod = [-0x401 << i - 10 for (i, bit) in enumerate(bin(lbj)[::-1]) if bit == '1']
    # Save it in native coords
    rfj_pos = [(1 + i - i // 9, 1 + (i + 8) - (i + 8) // 9) for (i, bit) in enumerate(bin(rfj)[::-1]) if bit == '1']
    lfj_pos = [(1 + i - i // 9, 1 + (i + 10) - (1 + i + 8) // 9) for (i, bit) in enumerate(bin(lfj)[::-1]) if
           bit == '1']
    rbj_pos = [(1 + i - i // 9, 1 + (i - 8) - (i - 8) // 9) for (i, bit) in enumerate(bin(rbj)[::-1]) if bit == '1']
    lbj_pos = [(1 + i - i // 9, 1 + (i - 10) - (i - 10) // 9) for (i, bit) in enumerate(bin(lbj)[::-1]) if bit == '1']

    # moves
    rf = 0b100001111111101111111101111111101111 ^ UNUSED_BITS
    lf = 0b100000111111110111111110111111110111 ^ UNUSED_BITS
    rb = 0b111110111111110111111110111111110000 ^ UNUSED_BITS
    lb = 0b111101111111101111111101111111100000 ^ UNUSED_BITS
    # code of jump
    rf_cod = [0x11 << i for (i, bit) in enumerate(bin(rf)[::-1]) if bit == '1']
    lf_cod = [0x21 << i for (i, bit) in enumerate(bin(lf)[::-1]) if bit == '1']
    rb_cod = [0x11 << i - 4 for (i, bit) in enumerate(bin(rb)[::-1]) if bit == '1']
    lb_cod = [0x21 << i - 5 for (i, bit) in enumerate(bin(lb)[::-1]) if bit == '1']
    # Save it in native coords
    rf_pos = [(1 + i - i // 9, 1 + (i + 4) - (i + 4) // 9) for (i, bit) in enumerate(bin(rf)[::-1]) if bit == '1']
    lf_pos = [(1 + i - i // 9, 1 + (i + 5) - (i + 5) // 9) for (i, bit) in enumerate(bin(lf)[::-1]) if bit == '1']
    rb_pos = [(1 + i - i // 9, 1 + (i - 4) - (i - 4) // 9) for (i, bit) in enumerate(bin(rb)[::-1]) if bit == '1']
    lb_pos = [(1 + i - i // 9, 1 + (i - 5) - (i - 5) // 9) for (i, bit) in enumerate(bin(lb)[::-1]) if bit == '1']

    all_jumps_cod = rfj_cod + lfj_cod + rbj_cod + lbj_cod
    all_jumps_pos = rfj_pos + lfj_pos + rbj_pos + lbj_pos
    all_moves_cod = rf_cod + lf_cod + rb_cod + lb_cod
    all_moves_pos = rf_pos + lf_pos + rb_pos + lb_pos

    total_moves_cod = all_moves_cod + all_jumps_cod
    total_moves_pos = all_moves_pos + all_jumps_pos

    all_moves = []

    for move in total_moves_cod:
      if (move, True) in all_moves:
        all_moves.append((move, False))
      else:
        all_moves.append((move, True))

    return all_moves



class CheckersConsole:
  def on_genmove(self, batch, items, reply):
    reply["a"] = int(items[1])
    return True, reply

  def on_exit(self, batch, items, reply):
    self.exit = True
    return True, reply


  def __init__(self, GC, evaluator):
    self.exit = False
    self.GC = GC
    self.evaluator = evaluator
    self.moves_for_human = CheckerMoves.get_all_moves()

    self.commands = {
      key[3:]: func
      for key, func in inspect.getmembers(
        self, predicate=inspect.ismethod)
      if key.startswith("on_")
    }

  def valid_moves(self, batch):
    valid = batch.GC.getGame(0).getValidMoves()

    for idx in range(len(valid)):
      if valid[idx]:
        i = "{0:036b}".format(self.moves_for_human[idx][0])[::-1]
        index = [pos for pos, char in enumerate(i) if char == "1"]
        i1, i2 = index
        buff1 = (1 + i1 - i1 // 9) - 1
        buff2 = (1 + i2 - i2 // 9) - 1
        x1, y1 = (6 - (buff1) % 4 * 2 + ((buff1) // 4) % 2, 7 - (buff1) // 4)
        x2, y2 = (6 - (buff2) % 4 * 2 + ((buff2) // 4) % 2, 7 - (buff2) // 4)
        if not self.moves_for_human[idx][1]:
          x1, y1, x2, y2 = x2, y2, x1, y1

        print("", idx, "\t: ", (x1 + y1 * 8), "=>", (x2 + y2 * 8))
    print("")
    return True, None

  def actor(self, batch):
    reply = self.evaluator.actor(batch)
    return reply

  def list_commands(self, batch, items, reply):
    msg = "\n".join(self.commands.keys())
    return True, msg

  def prompt(self, prompt_str, batch):
    self.valid_moves(batch)

    while True:
      cmd = input(prompt_str)
      items = cmd.split()
      if len(items) < 1:
        print("\u001b[31;1mInvalid input\u001b[0m : ", cmd)
        continue

      c = items[0]
      reply = dict(pi=None, a=None, checkers_V=0)

      try:
        ret, msg = self.commands[c](batch, items, reply)
        if not ret:
          print(msg)
        else:
          if isinstance(msg, dict):
            return msg
          elif isinstance(msg, str):
            print(msg)
          else:
            print("")
        # if not self.exit:
        self.valid_moves(batch)

      except Exception:
        ret, msg = self.list_commands(batch, items, reply)
        print("\u001b[31;1mInvalid command\u001b[0m   : ", cmd, "\n")
        
        print("\u001b[34mmAvilable commands\u001b[0m :\n", msg)
