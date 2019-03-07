#!/usr/bin/env python

import sys
import inspect
import numpy as np
import json
from time import sleep

sys.path.append('..')
from py.CheckersMoves import get_all_moves

class CheckersGui():
  def __init__(self, GC, evaluator, connection, sid):
    self.exit = False
    self.GC = GC
    self.checkers_board_size = GC.params["checkers_board_size"]
    self.evaluator = evaluator
    self.moves_for_human = get_all_moves()

    self.connection = connection
    self.sid = sid
    self.message = None

    self.RESET_CODE = -1
    self.EXIT_CODE = -2
    print("Connection was established")

  def get_valid_moves(self, batch):
    valid_moves = batch.GC.getGame(0).getValidMoves()
    move_pairs_dict = {}

    for idx in range(len(valid_moves)):
      if valid_moves[idx]:
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

        move_pairs_dict[idx] = [(x1, y1), (x2, y2)]
    print("")
    return move_pairs_dict

  def act_model(self, batch):
    reply = self.evaluator.actor(batch)
    return reply

  def act_human(self, batch):
    valid_moves = batch.GC.getGame(0).getValidMoves()
    self.send_state(batch, None)
    while True:
      if self.exit:
        print("game interrupted!")
        return self.EXIT_CODE

      if self.message is None:
        sleep(0.2)
        continue

      try:
        print("received message: \n", self.message)
        json_message = json.loads(self.message)
      except:
        print("Could not parse json message")
        self.send_error("Could not parse json message")
        self.message = None
        continue

      if json_message["type"] == "get_state":
        self.send_state(batch, None)
      elif json_message["type"] == "action":
        data = json_message["data"]
        try:
          action_idx = int(data)
          if action_idx == self.RESET_CODE:
            break
          elif action_idx == self.EXIT_CODE:
            self.exit = True
            break
          elif valid_moves[action_idx]:
            break
          else:
            self.send_error('Invalid move index')
        except ValueError:
          self.send_error("Could not parse move index")
      else:
        self.send_error("Could not parse move index")

      self.message = None
    print("received action ", action_idx)
    self.message = None

    # Fill our memmory
    
    reply = dict(pi=None, a=None, checkers_V=0)
    reply["a"] = int(action_idx)
    return reply



  # GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI
  # GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI
  # GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI GUI
  def get_observation(self, batch):
    data = batch.GC.getGame(0).getBoard()
    data = np.array(data.split()).astype(np.float).reshape(8,8)
    return data


  def send_state(self, batch, game_player):
    print("sending game info...")
    canonical_state = self.get_observation(batch)
    move_pairs_dict = self.get_valid_moves(batch)

    data = {
      'type': 'game_state',
      'data': {
        'state': canonical_state.tolist(),
        'moves': move_pairs_dict
      },
      'sid': self.sid
    }

    json_data = json.dumps(data)

    self.send_message(json_data.encode("utf-8"))

    print("game info sent!")

  def send_error(self, message):
    data = {'type': 'error', 'data': message, 'sid': self.sid}
    json_data = json.dumps(data)
    self.send_message(json_data.encode("utf-8"))

  def send_message(self, message):
    print("send_message ", message)
    self.connection.sendMessage(message)

  def message_control(self, data):
    self.message = data
