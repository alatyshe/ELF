# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import inspect
import traceback
from collections import Counter

from UgolkiMoves import get_all_moves_ugolki

class UgolkiConsole:
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
    self.moves_for_human = get_all_moves_ugolki()

    self.commands = {
      key[3:]: func
      for key, func in inspect.getmembers(
        self, predicate=inspect.ismethod)
      if key.startswith("on_")
    }

  def actor(self, batch):
    reply = self.evaluator.actor(batch)
    return reply

  def prompt(self, prompt_str, batch):
    self.print_valid_moves(batch)

    while True:
      cmd = input(prompt_str)
      items = cmd.split()
      if len(items) < 1:
        print("\u001b[31;1mInvalid input\u001b[0m : ", cmd)
        continue

      c = items[0]
      reply = dict(pi=None, a=None, V=0)

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
        self.print_valid_moves(batch)

      except Exception:
        ret, msg = self.list_commands(batch, items, reply)
        print("\u001b[31;1mInvalid command\u001b[0m   : ", cmd, "\n")    
        print("\u001b[34mmAvilable commands\u001b[0m :\n", msg)

  def print_valid_moves(self, batch):
    valid_moves = batch.GC.getGame(0).getValidMoves()
    for idx in range(len(valid_moves)):
      if valid_moves[idx]:
        move = self.moves_for_human[idx]
        print(f"{idx}\t: {move[0]} => {move[1]}")
    print("")

  def list_commands(self, batch, items, reply):
    msg = "\n".join(self.commands.keys())
    return True, msg
