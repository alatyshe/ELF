#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import os
import sys
import time
import re
from datetime import datetime

import torch

from elf import logging
from rlpytorch import \
  Evaluator, load_env, ModelInterface

logger = logging.getIndexedLogger(
  '\u001b[31;1m|py|\u001b[0melfgames.american_checkers.selfplay-',
  '')

class Stats(object):
  def __init__(self):
    self.total_batchsize = 0
    self.total_sel_batchsize = 0
    self.actor_count = 0
    logger = logging.getIndexedLogger(
      '\u001b[31;1m|py|\u001b[0melfgames.american_checkers.Stats-',
      '')

  def feed(self, batch):
    self.total_sel_batchsize += batch.batchsize
    self.total_batchsize += batch.max_batchsize
    self.actor_count += 1

    if self.total_sel_batchsize >= 500000:
      logger.info("")

      batch_usage = self.total_sel_batchsize / self.total_batchsize
      # wr = batch.GC.getClient().getGameStats().getWinRateStats()
      # win_rate = (100.0 * wr.black_wins / (wr.black_wins + wr.white_wins)
      #       if (wr.black_wins + wr.white_wins) > 0
      #       else 0.0)

      print(f'\tBatch usage:'
          f'\t{self.total_sel_batchsize}/{self.total_batchsize} '
          f'\t({100.0 * batch_usage:.2f}%)')


      # print(f'\tB/W: {wr.black_wins}/{wr.white_wins}, ', end="")
      # print(f'Both Lost: {wr.both_lost}, ', end="")
      # print(f'Black winrate: {win_rate:.2f}, ', end="")
      # print(f'Total Games:{wr.total_games}')

      self.total_sel_batchsize = 0
      self.total_batchsize = 0
      print('\tActor count:\t', self.actor_count)



name_matcher = re.compile(r"save-(\d+)")

def extract_ver(model_loader):
  name = os.path.basename(model_loader.options.load)
  m = name_matcher.match(name)
  return int(m.group(1))

def reload_model(model_loader, params, mi, actor_name, args):
  model = model_loader.load_model(params)

  if actor_name not in mi:
    mi.add_model(actor_name, model, cuda=(args.gpu >= 0), gpu_id=args.gpu)
  else:
    mi.update_model(actor_name, model)
  mi[actor_name].eval()

def reload(mi, model_loader, params, args, root, ver, actor_name):
  if model_loader.options.load is None or model_loader.options.load == "":
    real_path = os.path.join(root, "save-" + str(ver) + ".bin")
    # print(f'\u001b[31;1m|py|\u001b[0m\x1b[0;33;40mModel for {actor_name} is loading from {real_path}\u001b[0m')
  else:
    this_root = os.path.dirname(model_loader.options.load)
    real_path = os.path.join(this_root, "save-" + str(ver) + ".bin")
    # print(f'\u001b[31;1m|py|\u001b[0m\x1b[0;33;40mLoad model for {actor_name}: {real_path}\u001b[0m')

  if model_loader.options.load != real_path:
    model_loader.options.load = real_path
    reload_model(model_loader, params, mi, actor_name, args)
  # else:
    # print('\u001b[31;1m|py|\u001b[0m\u001b[31;1mWarning! Same model, skip loading\u001b[0m', real_path)



def main():
  print('Python version:', sys.version)
  print('PyTorch version:', torch.__version__)
  print('CUDA version', torch.version.cuda)
  print('Conda env:', os.environ.get("CONDA_DEFAULT_ENV", ""))

  # Register player names
  actors = ["actor_white", "actor_black"]

  """
    Class Evaluator is a pure python class, 
    which run neural network in eval mode and get 
    return results and update some stat info.
    Will creates 'eval_actor_white', 'eval_actor_black'.
  """
  additional_to_load = {
    ("eval_" + actor_name): (
      Evaluator.get_option_spec(name="eval_" + actor_name),
      lambda object_map, actor_name=actor_name: 
        Evaluator(
          object_map, name="eval_" + actor_name,
          actor_name=actor_name, 
          stats=None)
    )
    for i, actor_name in enumerate(actors)
  }

  """
    class ModelInterface is a python class saving network models.
    Its member models is a key-value store to call a CNN model by name.
    Will creates 'mi_actor_white', 'mi_actor_black'.
  """
  additional_to_load.update({
    ("mi_" + name): (
      ModelInterface.get_option_spec(), 
      ModelInterface)
    for name in actors
  })

  """
    load_env:
    game - load file game elfgames.american_checkers.game
    method - load "method" passed via params:
        file model_american_checkers.py return array with [model, method]
        model_file=elfgames.american_checkers.model_american_checkers
        model=df_pred 
    model_loaders - prepare to load(returns instance of class ModelLoader)
        "model" passed via params:
        file model_american_checkers.py return array with [model, method]
        model_file=elfgames.american_checkers.model_american_checkers
        model=df_pred
    
    sampler - Used to sample an action from policy.
    mi - class ModelInterface is a python class saving network models.
        Its member models is a key-value store to call a CNN model by name.
    eval_* - run neural network in eval mode and get 
        return results and update some stat info.
  """
  env = load_env(
    os.environ, 
    num_models=2, 
    overrides={'actor_only': True},
    additional_to_load=additional_to_load)

  """
    Initializes keys('game_end', 'game_start', 'actor_white', 'actor_black')
    for communication Python and C++ code, defined in Game.py and GameFeature.h.
    Also, initializes GameContext from C++ library wrapped by GC from python side
    + sets mode that parsed from options like play/selfplay/train/offline_train.
  """
  GC = env["game"].initialize()

  """
    Registering the methods in the GameContext on the python side.
    We registered their names earlier when the game was 
    initialized(names were registered on the python and C++ sides).
    Now its a registration of methods that will be called 
    when we try to pass batch on eval from C++ to Python.
    Example:
      We register "human_actor" as key and register the 
      same method on the python side. 
      When AIClientT calls method act(it takes 2 parameters: state, and key)
      act connect to python and transmits the state by 
      key("human_actor", "actor_black")
      to these methods(actor() func defined below).
  """
  # Some statistic about batch usage, also we can add more info about games stats.
  stats = [Stats(), Stats()]

  for i in range(len(actors)):
    actor_name = actors[i]
    stat = stats[i]

    evaluator = env["eval_" + actor_name]
    evaluator.setup(sampler=env["sampler"], mi=env["mi_" + actor_name])

    def actor(batch, evaluator, stat):
      reply = evaluator.actor(batch)
      stat.feed(batch)
      return reply
    # To expand the functionality we use lambda
    GC.reg_callback(actor_name,
            lambda batch, evaluator=evaluator, stat=stat: actor(batch, evaluator, stat))


  # Get the directory containing the models.
  root = os.environ.get("root", "./")
  args = env["game"].options
  # Stops client after N games, defined in --suicide_after_n_games param.
  loop_end = False

  """
    This method is responsible for updating the model to the 
    current one(received from the server) after starting. 
    Called by 'game_start' key from C++ side.
  """
  def game_start(batch):
    info = "game_start() load/reload models\n"
    logger.info(info)

    vers = [
        int(batch["white_ver"][0]),
        int(batch["black_ver"][0])
        ]

    # Use the version number to load models.
    for model_loader, ver, actor_name in zip(
        env["model_loaders"], vers, actors):
      if ver >= 0:
        while True:
          try:
            reload(
              env["mi_" + actor_name], model_loader, GC.params,
              args, root, ver, actor_name)
            break
          except BaseException:
            import traceback
            traceback.print_exc()
            time.sleep(10)

  """
    This method is responsible for displaying game statistics, 
    as well as stopping the client after N games(loop_end).
    Called by 'game_end' key from C++ side.
  """
  def game_end(batch):
    nonlocal loop_end
    wr = batch.GC.getClient().getGameStats().getWinRateStats()
    win_rate = (100.0 * wr.black_wins / (wr.black_wins + wr.white_wins)
          if (wr.black_wins + wr.white_wins) > 0 else 0.0)

    info =  f'game_end()\tB/W: {wr.black_wins}/{wr.white_wins}, '
    info += f'Draw: {wr.both_lost}, '
    info += f'Black winrate: {win_rate:.2f}, '
    info += f'Total Games: {wr.total_games}'

    logger.info(info)
    if args.suicide_after_n_games > 0 and \
        wr.total_games >= args.suicide_after_n_games:
      info = f'game_end()\tTotal Games: {wr.total_games}, '
      info += f'#suicide_after_n_games: {args.suicide_after_n_games}'
      logger.info(info)
      loop_end = True

  # Registering the methods described above in Python's GameContext.
  GC.reg_callback_if_exists("game_start", game_start)
  GC.reg_callback_if_exists("game_end", game_end)

  GC.start()

  """
    Upon receiving the --eval_model_pair parameter, we load 2 models 
    from a file and pass models versions to C++ side for evaluation.
  """
  if args.eval_model_pair:
    if args.eval_model_pair.find(",") >= 0:
      black, white = args.eval_model_pair.split(",")
    else:
      black = extract_ver(env["model_loaders"][0])
      white = extract_ver(env["model_loaders"][1])
      # Force them to reload in the future.
      for model_loader, actor_name in zip(env["model_loaders"], actors):
        reload_model(model_loader, GC.params,
               env["mi_" + actor_name], actor_name, args)

    # We just use one thread to do selfplay.
    GC.GC.getClient().setRequest(
      int(black), int(white), 1)

  # Called before each episode, resets actor_count(num of total nn call)
  for actor_name in actors:
    env["eval_" + actor_name].episode_start(0)

  while not loop_end:
    GC.run()

  GC.stop()


if __name__ == '__main__':
  main()
