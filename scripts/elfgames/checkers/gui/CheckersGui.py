#!/usr/bin/env python

import inspect
import numpy as np
import json
from time import sleep

def even(num):
  return num % 2 == 0

def game_piece(x, y):
  return (even(y) and (not even(x))) or ((not even(y)) and even(x))

def _get_all_moves_from(position, size):
  end_positions = [
    (position[0] - 1, position[1] - 1),
    (position[0] - 1, position[1] + 1),
    (position[0] + 1, position[1] - 1),
    (position[0] + 1, position[1] + 1),
    (position[0] - 2, position[1] - 2),
    (position[0] - 2, position[1] + 2),
    (position[0] + 2, position[1] - 2),
    (position[0] + 2, position[1] + 2),
  ]
  moves = []
  for end_position in end_positions:
    if 0 <= end_position[0] < size and 0 <= end_position[1] < size:
      moves.append([position, end_position])
  return moves

def get_all_moves_old():
  moves = []
  size = 8
  bw_state = [x[:] for x in [[0] * size] * size]
  for y, row in enumerate(bw_state):
    for x, piece in enumerate(row):
      if game_piece(x, y):
        position = (x, y)
        moves.extend(_get_all_moves_from(position, size))

  return moves


class CheckerMoves:
  @staticmethod
  def get_all_moves_new():
    UNUSED_BITS = 0b100000000100000000100000000100000000

    rfj = 0b100000000111101110111101110111101110 ^ UNUSED_BITS
    lfj = 0b100000000101110111101110111101110111 ^ UNUSED_BITS
    rbj = 0b101110111101110111101110111100000000 ^ UNUSED_BITS
    lbj = 0b111101110111101110111101110100000000 ^ UNUSED_BITS

    rfj_cod = [-0x101 << i for (i, bit) in enumerate(bin(rfj)[::-1]) if bit == '1']
    lfj_cod = [-0x401 << i for (i, bit) in enumerate(bin(lfj)[::-1]) if bit == '1']
    rbj_cod = [-0x101 << i - 8 for (i, bit) in enumerate(bin(rbj)[::-1]) if bit == '1']
    lbj_cod = [-0x401 << i - 10 for (i, bit) in enumerate(bin(lbj)[::-1]) if bit == '1']

    rf = 0b100001111111101111111101111111101111 ^ UNUSED_BITS
    lf = 0b100000111111110111111110111111110111 ^ UNUSED_BITS
    rb = 0b111110111111110111111110111111110000 ^ UNUSED_BITS
    lb = 0b111101111111101111111101111111100000 ^ UNUSED_BITS

    rf_cod = [0x11 << i for (i, bit) in enumerate(bin(rf)[::-1]) if bit == '1']
    lf_cod = [0x21 << i for (i, bit) in enumerate(bin(lf)[::-1]) if bit == '1']
    rb_cod = [0x11 << i - 4 for (i, bit) in enumerate(bin(rb)[::-1]) if bit == '1']
    lb_cod = [0x21 << i - 5 for (i, bit) in enumerate(bin(lb)[::-1]) if bit == '1']

    all_jumps_cod = rfj_cod + lfj_cod + rbj_cod + lbj_cod
    all_moves_cod = rf_cod + lf_cod + rb_cod + lb_cod

    total_moves_cod = all_moves_cod + all_jumps_cod

    all_moves = []

    for move in total_moves_cod:
      if (move, True) in all_moves:
        all_moves.append((move, False))
      else:
        all_moves.append((move, True))

    return all_moves


class CheckersGui():

  def on_genmove(self, batch, items, reply):
    reply["a"] = int(items[1])
    return True, reply

  def on_play(self, batch, items, reply):
    ret, msg = self.check_player(batch, items[1][0])
    if ret:
      return True, reply
    else:
      return False, msg

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

  def on_quit(self, batch, items, reply):

    self.exit = True
    return True, reply

  def __init__(self, GC, evaluator, connection, sid,
         send_mcts_stats_enabled=False):

    # ELF PARAMS
    self.exit = False
    self.GC = GC
    self.checkers_board_size = GC.params["checkers_board_size"]
    self.evaluator = evaluator
    self.last_cmd = ""

    self.moves_for_human = CheckerMoves.get_all_moves_new()
    self.all_moves = get_all_moves_old()

    self.actions = {}

    for idx, move in enumerate(self.all_moves):
      self.actions[idx] = move

    self.commands = {
      key[3:]: func
      for key, func in inspect.getmembers(
      self, predicate=inspect.ismethod)
      if key.startswith("on_")
    }

    # RL_CHECKERS PARAMS

    self.connection = connection
    self.sid = sid
    self.send_mcts_stats_enabled = send_mcts_stats_enabled

    self.victory = None
    self.RESET_GAME_CODE = -1
    self.INTERRUPT_GAME_CODE = -2
    self.interrupted = False
    self.message = None
    self.last_player = None

    print("Connection was established")

  # ELF part

  def actor(self, batch):
    reply = self.evaluator.actor(batch)
    return reply

  def list_commands(self, batch, items, reply):
    msg = "\n".join(self.commands.keys())
    return True, msg

  def prompt(self, prompt_str, batch):
    # Show last command results.
    if self.last_cmd == "play" or self.last_cmd == "clear_board":
      print("New Game")

    self.last_cmd = ""

    valid_moves = self.get_valid_moves(batch)

    self.send_state(batch, None)

    while (not self.isEnded(batch)):

      if self.interrupted:
        self.exit = True
        print("game interrupted!")
        return self.INTERRUPT_GAME_CODE

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

          if action_idx == self.RESET_GAME_CODE:
            break
          elif action_idx == self.INTERRUPT_GAME_CODE:
            self.interrupted = True
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


    cmd = "genmove " + str(action_idx)
    self.message = None

    items = cmd.split()

    c = items[0]
    reply = dict(pi=None, a=None, checkers_V=0)

    try:
      ret, msg = self.commands[c](batch, items, reply)
      self.last_cmd = c
      if not ret:
        print(msg)
      else:
        if isinstance(msg, dict):
          return msg
        elif isinstance(msg, str):
          print(msg)
        else:
          print("")

    except Exception:

      ret, msg = self.list_commands(batch, items, reply)
      print("\u001b[31;1mInvalid command\u001b[0m   : ", cmd, "\n")

      print("\u001b[34mmAvilable commands\u001b[0m :\n", msg)


  def isEnded(self, batch):

    isEnded = batch.GC.getGame(0).isEnded()

    return (isEnded)


  def get_observation(self, batch):

    data = batch.GC.getGame(0).getBoard()
    data = np.array(data.split()).astype(np.float).reshape(8,8)

    return (data)

  def get_all_moves(self, batch):

    return (self.all_moves)

  def get_valid_moves(self, batch):

    valid_moves = batch.GC.getGame(0).getValidMoves()

    return (valid_moves)


  # RL_CHECKERS PART

  def prepare_to_game(self):
    pass

  def send_state(self, batch, game_player):
    print("sending game info...")

    valid_moves = self.get_valid_moves(batch)
    canonical_state = self.get_observation(batch)
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

        move_pairs_dict[idx] = [(x1, y1), (x2, y2)]

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

  def interrupt(self):
    self.interrupted = True

  def on_message(self, data):
    self.message = data
