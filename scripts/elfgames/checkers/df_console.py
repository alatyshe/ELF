#!/usr/bin/env python

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import os
import sys

import torch

# оборачиваем в наш переписанный для терминалаы GameContext
from console_lib import GoConsoleGTP
from rlpytorch import Evaluator, load_env


def main():
    print('Python version:', sys.version)
    print('PyTorch version:', torch.__version__)
    print('CUDA version', torch.version.cuda)
    print('Conda env:', os.environ.get("CONDA_DEFAULT_ENV", ""))

    additional_to_load = {
        'evaluator': (
            Evaluator.get_option_spec(),
            lambda object_map: Evaluator(object_map, stats=None)),
    }

    # Set game to online model.
    print("\t\t\tENV : ")
    env = load_env(
        os.environ,
        overrides={
            'num_games': 1,
            'greedy': True,
            'T': 1,
            'model': 'online',
            'additional_labels': ['aug_code', 'move_idx'],
        },
        additional_to_load=additional_to_load)

    evaluator = env['evaluator']

    print("ENV : ")
    for i in env:
        print(i, "\t : ", env[i])
    print("\t\t\tGC : ")
    GC = env["game"].initialize()

    print("model_loader : ")
    model_loader = env["model_loaders"][0]
    
    print("\t\t\tmodel : ")
    model = model_loader.load_model(GC.params)

    print("\t\t\tmi : ")
    mi = env['mi']
    mi.add_model("model", model)
    mi.add_model("actor", model)
    mi["model"].eval()
    mi["actor"].eval()

    print("\t\t\tconsole : ")
    # оборачиваем в наш переписанный для терминалаы GameContext
    console = GoConsoleGTP(GC, evaluator)

    def human_actor(batch):
        print("HUMAN : ")
        return console.prompt("", batch)

    def actor(batch):
        print("ACTOR : ")
        return console.actor(batch)

    def train(batch):
        print("TRAIN : ")
        console.prompt("DF Train> ", batch)

    evaluator.setup(sampler=env["sampler"], mi=mi)

    GC.reg_callback_if_exists("actor_black", actor)
    GC.reg_callback_if_exists("human_actor", human_actor)
    GC.reg_callback_if_exists("train", train)

    GC.start()
    GC.GC.getClient().setRequest(
        mi["actor"].step, -1, env['game'].options.resign_thres, -1)

    evaluator.episode_start(0)

    while True:
        GC.run()
        if console.exit:
            break
    GC.stop()


if __name__ == '__main__':
    main()
