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

    def human_actor(batch):
      return self.gui.act_human(batch)

    def actor(batch):
      return self.gui.act_model(batch)

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
      if self.gui.exit:
        break
    # fix this for normal exit
    sys.exit()

    GC.stop()





if __name__ == "__main__":
  game_storage = {}

  """
    Class Evaluator is a pure python class, 
    which run neural network in eval mode and get 
    return results and update some stat info
  """
  additional_to_load = {
    'evaluator': (
      Evaluator.get_option_spec(),
      lambda object_map: Evaluator(object_map, stats=None)),
  }

  """
    load_env:
    game - load file game elfgames.checkers.game
    method - load "method" passed via params:
        file df_model_checkers.py return array with [model, method]
        model_file=elfgames.checkers.df_model_checkers
        model=df_pred 
    model_loaders - prepare to load(returns instance of class ModelLoader)
        "model" passed via params:
        file df_model_checkers.py return array with [model, method]
        model_file=elfgames.checkers.df_model_checkers
        model=df_pred
    
    sampler - Used to sample an action from policy.
    mi - class ModelInterface is a python class saving network models.
        Its member models is a key-value store to call a CNN model by name.
    evaluator - run neural network in eval mode and get 
        return results and update some stat info.
  """
  env = load_env(
    os.environ,
    overrides={
      'num_games': 1,
      'greedy': True,
      'T': 1,
      'additional_labels': ['checkers_aug_code', 'checkers_move_idx'],
    },
    additional_to_load=additional_to_load)

  evaluator = env['evaluator']
  """
    Initializes keys for communication Python and C++ code, 
    defined in Game.py and GameFeature.h.
    Also, initializes GameContext from C++ library wrapped by GC from python side
    + sets mode that parsed from options like play/selfplay/train/offline_train.
  """
  GC = env["game"].initialize()

  # Load model(use Model_PolicyValue from df_model_checkers.py)
  model_loader = env["model_loaders"][0]
  # Model contains init_conv, value_func, resnet and etc.
  model = model_loader.load_model(GC.params)

  """
    Pass our model in ModelInterface
    ModelInterface stores our saved model and call nn when we need eval 
  """
  mi = env['mi']
  mi.add_model("actor", model)
  # Checking the success installed model
  mi["actor"].eval()

  try:
    port = 8888
    server = SimpleWebSocketServer('', port, WebSocketServerClient)

    print("listening for incoming connections...")
    server.serveforever()
    server.start()
  except:
    print("Could not create socket.")
    sys.exit(0)

  
