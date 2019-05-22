#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import inspect
import os
import sys
import re
import time
import torch

from elf import logging
from rlpytorch import load_env, SingleProcessRun, Trainer

matcher = re.compile(r"save-(\d+).bin")

def main():
  print('Python version:', sys.version)
  print('PyTorch version:', torch.__version__)
  print('CUDA version', torch.version.cuda)
  print('Conda env:', os.environ.get("CONDA_DEFAULT_ENV", ""))

  logger = logging.getIndexedLogger(
    '\u001b[31;1m|py|\u001b[0melfgames.checkers.train-',
    '')

  # Trainer is also a pure python class wrapped on evaluator.
  # Train the models.
  # Runner - seems run all this shit.
  additional_to_load = {
    'trainer': (
      Trainer.get_option_spec(),
      lambda option_map: Trainer(option_map)),
    'runner': (
      SingleProcessRun.get_option_spec(),
      lambda option_map: SingleProcessRun(option_map)),
  }



  env = load_env(os.environ, additional_to_load=additional_to_load)





  trainer = env['trainer']
  runner = env['runner']



  """
    Initializes keys('train', 'train_ctrl')
    for communication Python and C++ code, defined in Game.py and GameFeature.h.
    Also, initializes GameContext from C++ library wrapped by GC from python side
    + sets mode that parsed from options like play/selfplay/train/offline_train.
  """
  GC = env["game"].initialize()


  model_loader = env["model_loaders"][0]
  model = model_loader.load_model(GC.params)
  env["mi"].add_model("model", model, opt=True)


  keep_prev_selfplay = env["game"].options.keep_prev_selfplay
  model_ver = 0

  # Грузим модель, если указана
  model_filename = model_loader.options.load
  if isinstance(model_filename, str) and model_filename != "":
    realpath = os.path.realpath(model_filename)
    m = matcher.match(os.path.basename(realpath))
    if m:
      model_ver = int(m.group(1))

  eval_old_model = env["game"].options.eval_old_model

  # 
  if eval_old_model >= 0:
    GC.GC.getServer().setEvalMode(model_ver, eval_old_model)
  else:
    GC.GC.getServer().setInitialVersion(model_ver)

  checkers_selfplay_ver = model_ver
  root = os.environ["save"]

  print(f'Save models in\t\t: "{root}"')
  print(f'Keep prev_selfplay\t: {keep_prev_selfplay!s}')


  def train(batch, *args, **kwargs):
    # Check whether the version match.
    if keep_prev_selfplay or \
        (batch["checkers_selfplay_ver"] != checkers_selfplay_ver).sum() == 0:
      trainer.train(batch, *args, **kwargs)
    else:
      print(f'Get batch whose selfplay ver is different from '
          f'{checkers_selfplay_ver}, skipping')
      runner.inc_episode_counter(-1)


  def train_ctrl(batch, *args, **kwargs):
    nonlocal checkers_selfplay_ver
    
    old_selfplay_ver = checkers_selfplay_ver
    checkers_selfplay_ver = int(batch["checkers_selfplay_ver"][0])
    logger.info(
      f'Train ctrl: checkers_selfplay_ver: {old_selfplay_ver} -> {checkers_selfplay_ver}')
    
    # ожидаем нормально запоненого батча от клиентов
    GC.GC.getServer().ServerWaitForSufficientSelfplay(checkers_selfplay_ver)

    # Reload old models.
    real_path = os.path.join(root, "save-" + str(checkers_selfplay_ver) + ".bin")
    model_loader.options.load = real_path

    while True:
      try:
        model = model_loader.load_model(GC.params)
        break
      except BaseException:
        time.sleep(10)

    env["mi"].remove_model("model")
    env["mi"].add_model("model", model, opt=True)
    trainer.episode_reset()
    runner.set_episode_counter(-1)



  GC.reg_callback("train", train)
  GC.reg_callback("train_ctrl", train_ctrl)

  if GC.reg_has_callback("actor"):
    args = env["game"].options
    env["mi"].add_model(
      "actor",
      model,
      copy=True,
      cuda=(args.gpu >= 0),
      gpu_id=args.gpu)
    GC.reg_callback("actor", trainer.actor)



  # +++++++++++++++++++++++++++++++++++++++++++++++
  # +++++++++++++++++++++++++++++++++++++++++++++++
  # +++++++++++++++++++++++++++++++++++++++++++++++
  trainer.setup(
    sampler=env["sampler"],
    mi=env["mi"],
    rl_method=env["method"])

  def episode_summary(i):
    nonlocal checkers_selfplay_ver

    logger.info("Episode_summary")
    ver = trainer.episode_summary(i)
    # This might block (when evaluation does not catch up with training).
    GC.GC.getServer().notifyNewVersion(checkers_selfplay_ver, ver)

  offline_training = (env["game"].options.mode == "offline_train")

  def after_start():
    logger.info("after_start")

    nonlocal checkers_selfplay_ver
    if not offline_training:
      GC.GC.getServer().ServerWaitForSufficientSelfplay(checkers_selfplay_ver)

  # sys.exit(0)

  runner.setup(GC, after_start=       after_start,
          episode_summary=    episode_summary,
          episode_start=      trainer.episode_start)

  runner.run_singe_process()


if __name__ == '__main__':
  main()
