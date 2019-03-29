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
  game_storage = {}

  def handleConnected(self):
    self.connection = self

  def handleMessage(self):
    print("\n\nhandleMessage  :\t", self.data)

    # Работающее в потоке gui, передаем ему таким способом дату
    if self.gui:
      self.gui.pass_message(self.data)
      return

    if self.connection:
      print("self.connection != None")
      json_message = json.loads(self.data)

      if json_message["type"] == "get_state":
        # get id of game
        if "sid" in json_message and json_message["sid"] in self.game_storage:
          pass

        print("build new game")
        sid = "sid_" + uuid.uuid4().hex[:8]
        start_new_thread(self.build_game, (GC, sid))


  def handleClose(self):
    print("\n\nhandleClose")

    if self.gui is not None:
      print("self.gui is not None")
      self.gui.exit()
      print("fuck")


  def build_game(self, GC, sid):
    print("\n\nbuild game")

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

  
