#!/usr/bin/env python

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import os
import sys

import torch

from UgolkiConsole import UgolkiConsole
from rlpytorch import Evaluator, load_env

def main():
  print('Python version:', sys.version)
  print('PyTorch version:', torch.__version__)
  print('CUDA version', torch.version.cuda)
  print('Conda env:', os.environ.get("CONDA_DEFAULT_ENV", ""))

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
      'additional_labels': ['aug_code', 'move_idx'],
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

  # Describe more!
  console = UgolkiConsole(GC, evaluator)

  def human_actor(batch):
    return console.prompt("", batch)

  def actor(batch):
    return console.actor(batch)

  evaluator.setup(sampler=env["sampler"], mi=mi)


  """
    Register the methods in the GameContext on the python side. 
    We registered their names earlier when the game was 
    initialized(names were registered on the python and C++ sides).
    Now its a registration of methods that will be called 
    when we try to pass batch on eval from C++ to Python.
    Example:
      We register "human_actor" as key and register the 
      same method on the python side. 
      When our AIClientT calls method act(it takes 2 parameters: state, and key)
      act connect to python and transmits the state by 
      key("human_actor", "actor_black")
      to these methods
  """
  GC.reg_callback_if_exists("human_actor", human_actor)
  GC.reg_callback_if_exists("actor_black", actor)
  GC.start()
  # Tells the С++ side the model version
  GC.GC.getClient().setRequest(
    mi["actor"].step, -1, -1)

  # Called before each episode, resets actor_count(num of total nn call)
  evaluator.episode_start(0)

  while True:
    GC.run()
    if console.exit:
      break
  
  # fix this for normal exit
  # sys.exit()
  
  GC.stop()


if __name__ == '__main__':
  main()
