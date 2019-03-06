#!/usr/bin/env python

import os
import sys

import json
import uuid

from rlpytorch import Evaluator, load_env

from _thread import start_new_thread
from CheckersGui import CheckersGui
from SimpleWebSocketServer import SimpleWebSocketServer, WebSocket

class WebSocketServerClient(WebSocket):
  gui = None
  connection = None

  def handleMessage(self):
    print("handle message: ", self.data)

    print(self.gui)

    if self.gui is not None:
      self.gui.message_control(self.data)
      return

    if self.connection:
      json_message = json.loads(self.data)

      if json_message["type"] == "get_state":
        if "sid" in json_message:
          sid = json_message["sid"]
          if sid in game_storage:
            print("load game from game storage")
            start_new_thread(self.build_game, (game_storage[sid], sid))
            return

        print("build new game")

        sid = self.generate_new_sid()
        game_storage[sid] = GC

        start_new_thread(self.build_game, (GC, sid))

  def generate_new_sid(self):
    return "sid_" + uuid.uuid4().hex[:8]

  def handleConnected(self):
    print(self.address, ' connected')
    self.connection = self

  def handleClose(self):
    if self.gui is not None:
      self.gui.interrupt()

  def build_game(self, GC, sid):
    print("build game")

    self.gui = CheckersGui(GC, evaluator, self.connection, sid)

    # console = ConsoleGTP(GC, evaluator)

    gui = self.gui

    def human_actor(batch):
      return gui.prompt(batch)

    def actor(batch):
      return gui.actor(batch)

    evaluator.setup(sampler=env["sampler"], mi=mi)

    GC.reg_callback_if_exists("human_actor", human_actor)
    GC.reg_callback_if_exists("checkers_actor_black", actor)
    GC.start()
    # Tells the С++ side the model version
    GC.GC.getClient().setRequest(
      mi["actor"].step, -1, -1)

    # Called before each episode, resets actor_count(num of total nn call)
    evaluator.episode_start(0)

    while True:
      GC.run()
      if gui.exit:
        break
    # fix this for normal exit
    sys.exit()

    GC.stop()



if __name__ == "__main__":
  game_storage = {}

  additional_to_load = {
    'evaluator': (
      Evaluator.get_option_spec(),
      lambda object_map: Evaluator(object_map, stats=None)),
  }

  env = load_env(
    os.environ,
    overrides={
      'num_games': 1,
      'greedy': True,
      'T': 1,
      'model': 'online',
      'additional_labels': ['checkers_aug_code', 'checkers_move_idx'],
    },
    additional_to_load=additional_to_load)

  evaluator = env['evaluator']

  GC = env["game"].initialize()

  model_loader = env["model_loaders"][0]
  model = model_loader.load_model(GC.params)

  mi = env['mi']
  mi.add_model("model", model)
  mi.add_model("actor", model)
  mi["model"].eval()
  mi["actor"].eval()

  port = 8888
  try:
    server = SimpleWebSocketServer('', port, WebSocketServerClient)

    print("listening for incoming connections...")
    server.serveforever()
    server.start()
  except:
    print("Could not create socket.")
    sys.exit(0)

  
