import inspect

import sys

import numpy as np
import json

from time import sleep

class Agent():

  def __init__(self, name="Agent"):
    self.name = name

  def get_name(self):
    return self.name

  def set_name(self, name):
    self.name = name

  def prepare_to_game(self):
    pass

  def predict(self, game, game_player):
    pass

  def on_turn_finished(self, game):
    pass

  def save(self, path_to_file):
    pass

  def load(self, path_to_file):
    pass

  def train(self, train_examples, batch_size = 2048, epochs = 10, verbose = 1):
    pass

  def clone(self):
    pass

  def set_exploration_enabled(self, enabled):
    pass


DEFAULT_BOARD_SIZE = 8

def is_even(num):
  return num % 2 == 0

def is_game_piece(x, y):
  return (is_even(y) and (not is_even(x))) or ((not is_even(y)) and is_even(x))

BLACK_PLAYER, WHITE_PLAYER = 1, -1

class CheckerBoard:
  def __init__(self, n=None, cloned=False):
    if n is not None:
      # TODO implement STARTING WHITES and STARTING BLACKS
      self.board_n = n
    else:
      self.board_n = DEFAULT_BOARD_SIZE

    self.BLACK_PLAYER, self.WHITE_PLAYER = 1, -1
    self.EMPTY, self.BLACK, self.BLACK_KING, self.WHITE, self.WHITE_KING = 0, 1, 3, -1, -3

    INITIAL_BOARD = [
      [(1, 0), self.WHITE], [(3, 0), self.WHITE], [(5, 0), self.WHITE], [(7, 0), self.WHITE],
      [(0, 1), self.WHITE], [(2, 1), self.WHITE], [(4, 1), self.WHITE], [(6, 1), self.WHITE],
      [(1, 2), self.WHITE], [(3, 2), self.WHITE], [(5, 2), self.WHITE], [(7, 2), self.WHITE],
      [(0, 5), self.BLACK], [(2, 5), self.BLACK], [(4, 5), self.BLACK], [(6, 5), self.BLACK],
      [(1, 6), self.BLACK], [(3, 6), self.BLACK], [(5, 6), self.BLACK], [(7, 6), self.BLACK],
      [(0, 7), self.BLACK], [(2, 7), self.BLACK], [(4, 7), self.BLACK], [(6, 7), self.BLACK]
    ]

    self.bw_state = None
    self.wb_state = None

    self.current_player = self.BLACK_PLAYER

    self.position_to_continue_jumps = None
    self.player_jumps = None
    self.all_moves = None

    if not cloned:
      self.bw_state = [x[:] for x in [[self.EMPTY] * self.board_n] * self.board_n]
      self.wb_state = [x[:] for x in [[self.EMPTY] * self.board_n] * self.board_n]

      for piece_obj in INITIAL_BOARD:
        position = piece_obj[0]
        piece = piece_obj[1]
        self.bw_state[position[1]][position[0]] = piece
        self.wb_state[7 - position[1]][7 - position[0]] = piece * (-1)

      self.get_all_moves()

  def get_all_moves(self):
    if self.all_moves is None:
      moves = []

      for y, row in enumerate(self.bw_state):
        for x, piece in enumerate(row):
          if is_game_piece(x, y):
            position = (x, y)
            moves.extend(self._get_all_moves_from(position))

      self.all_moves = moves

    return self.all_moves

  def _get_all_moves_from(self, position):
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
      if 0 <= end_position[0] < self.board_n and 0 <= end_position[1] < self.board_n:
        moves.append([position, end_position])

    return moves


class CheckerMoves:

  @staticmethod
  def get_all_moves():
    UNUSED_BITS = 0b100000000100000000100000000100000000

    rfj = 0b100000000111101110111101110111101110 ^ UNUSED_BITS
    lfj = 0b100000000101110111101110111101110111 ^ UNUSED_BITS
    rbj = 0b101110111101110111101110111100000000 ^ UNUSED_BITS
    lbj = 0b111101110111101110111101110100000000 ^ UNUSED_BITS

    rfj_cod = [-0x101 << i for (i, bit) in enumerate(bin(rfj)[::-1]) if bit == '1']
    lfj_cod = [-0x401 << i for (i, bit) in enumerate(bin(lfj)[::-1]) if bit == '1']
    rbj_cod = [-0x101 << i - 8 for (i, bit) in enumerate(bin(rbj)[::-1]) if bit == '1']
    lbj_cod = [-0x401 << i - 10 for (i, bit) in enumerate(bin(lbj)[::-1]) if bit == '1']

    # Save it in native coords
    rfj_pos = [(1 + i - i // 9, 1 + (i + 8) - (i + 8) // 9) for (i, bit) in enumerate(bin(rfj)[::-1]) if bit == '1']
    lfj_pos = [(1 + i - i // 9, 1 + (i + 10) - (1 + i + 8) // 9) for (i, bit) in enumerate(bin(lfj)[::-1]) if
           bit == '1']
    rbj_pos = [(1 + i - i // 9, 1 + (i - 8) - (i - 8) // 9) for (i, bit) in enumerate(bin(rbj)[::-1]) if bit == '1']
    lbj_pos = [(1 + i - i // 9, 1 + (i - 10) - (i - 10) // 9) for (i, bit) in enumerate(bin(lbj)[::-1]) if
           bit == '1']

    rf = 0b100001111111101111111101111111101111 ^ UNUSED_BITS
    lf = 0b100000111111110111111110111111110111 ^ UNUSED_BITS
    rb = 0b111110111111110111111110111111110000 ^ UNUSED_BITS
    lb = 0b111101111111101111111101111111100000 ^ UNUSED_BITS

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

    total_moves_cod = all_moves_cod + all_jumps_cod

    all_moves = []

    for move in total_moves_cod:
      if (move, True) in all_moves:
        all_moves.append((move, False))
      else:
        all_moves.append((move, True))

    return all_moves


class WrapperPlayerELF(Agent):

  def on_genmove(self, batch, items, reply):
    reply["a"] = int(items[1])
    return True, reply

  def on_play(self, batch, items, reply):
    ret, msg = self.check_player(batch, items[1][0])
    if ret:
      return True, reply
    else:
      return False, msg

  def on_board(self, batch, items, reply):

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
         debugNnetAgent=None,
         debugMCTSAgent=None,
         send_mcts_stats_enabled=False):

    super().__init__(name="Human via Web Socket Client")

    # ELF PARAMS
    self.exit = False
    self.GC = GC
    self.checkers_board_size = GC.params["checkers_board_size"]
    self.evaluator = evaluator
    self.last_cmd = ""
    self.moves_for_human = CheckerMoves.get_all_moves()

    self.CB = CheckerBoard()
    self.all_moves = self.CB.get_all_moves()
    print("all_moves all_moves all_moves all_moves all_moves all_moves all_moves : \n", self.all_moves)
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
    self.debugNnetAgent = debugNnetAgent
    self.debugMCTSAgent = debugMCTSAgent
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

  def showboard(self, batch):
    print(batch.GC.getGame(0).showBoard())

  def get_final_score(self, batch):
    return batch.GC.getGame(0).getLastScore()

  def print_msg(self, ret, msg):
    print("\n%s %s\n\n" % (("=" if ret else "?"), msg))

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

      self.on_board(batch, items, reply)

    except Exception:

      ret, msg = self.list_commands(batch, items, reply)
      print("\u001b[31;1mInvalid command\u001b[0m   : ", cmd, "\n")

      print("\u001b[34mmAvilable commands\u001b[0m :\n", msg)

  # WRAPPING part
  # Here called methods from C++ backend for using them in rl_checkers framework (see CheckersState.h and checkers/train/Pybind.cc)

  def killGame(self, batch):

    batch.GC.getGame(0).finish_game_for_server()

  def isEnded(self, batch):

    isEnded = batch.GC.getGame(0).isEnded()

    return (isEnded)

  def get_observation_player(self, batch):

    data = batch.GC.getGame(0).getObservationCurrentPlayer()

    return (np.asarray(data).astype(np.float))

  def get_observation(self, batch):

    data = batch.GC.getGame(0).getBoard()
    data = np.array(data.split()).astype(np.float).reshape(8,8)

    return (data)

  def get_all_moves(self, batch):

    return (self.all_moves)

  def get_valid_moves(self, batch):

    valid_moves = batch.GC.getGame(0).getValidMoves()

    return (valid_moves)

  def get_valid_moves_reverse(self, batch):

    valid_moves = batch.GC.getGame(0).getValidMovesReverse()

    return (valid_moves)

  def get_current_player(self, batch):

    player = batch.GC.getGame(0).getCurrentPlayer()

    return (player)

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

  def on_turn_finished(self, game):
    if self.send_mcts_stats_enabled:
      if self.debugNnetAgent is not None and not game.is_ended():
        self.send_action_estimator_data(game, self.last_player, self.debugNnetAgent,
                        'nnet_action_estimator_data')
      if self.debugMCTSAgent is not None and not game.is_ended():
        self.send_action_estimator_data(game, self.last_player, self.debugMCTSAgent,
                        'mcts_action_estimator_data')
  def on_game_ended(self, won):
    self.victory = won

    if not self.interrupted:
      data = {'type': 'end_game', 'sid': self.sid}
      if self.victory:
        data['data'] = 1
      else:
        data['data'] = 0


      json_data = json.dumps(data)

      self.send_message(json_data.encode("utf-8"))

    print("Game ended. Player won: ", self.victory)