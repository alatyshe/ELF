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
    '\x1b[1;31;40m|py|\x1b[0melfgames.checkers.selfplay-',
    '')

class Stats(object):
	def __init__(self):
		self.total_batchsize = 0
		self.total_sel_batchsize = 0
		self.actor_count = 0
		logger = logging.getIndexedLogger(
    		'\x1b[1;31;40m|py|\x1b[0melfgames.checkers.Stats-',
    		'')

	def feed(self, batch):
		self.total_sel_batchsize += batch.batchsize
		self.total_batchsize += batch.max_batchsize
		self.actor_count += 1

		if self.total_sel_batchsize >= 500000:
			logger.info("")

			batch_usage = self.total_sel_batchsize / self.total_batchsize
			# wr = batch.GC.getClient().getCheckersGameStats().getWinRateStats()
			# win_rate = (100.0 * wr.black_wins / (wr.black_wins + wr.white_wins)
			# 			if (wr.black_wins + wr.white_wins) > 0
			# 			else 0.0)

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
		print(f'\x1b[1;31;40m|py|\x1b[0m\x1b[0;33;40mModel for {actor_name} is loading from {real_path}\x1b[0m')
	else:
		this_root = os.path.dirname(model_loader.options.load)
		real_path = os.path.join(this_root, "save-" + str(ver) + ".bin")
		print(f'\x1b[1;31;40m|py|\x1b[0m\x1b[0;33;40mLoad model for {actor_name}: {real_path}\x1b[0m')

	if model_loader.options.load != real_path:
		model_loader.options.load = real_path
		reload_model(model_loader, params, mi, actor_name, args)
	else:
		print('\x1b[1;31;40m|py|\x1b[0m\x1b[1;31;40mWarning! Same model, skip loading\x1b[0m', real_path)





















def main():
	print('Python version:', sys.version)
	print('PyTorch version:', torch.__version__)
	print('CUDA version', torch.version.cuda)
	print('Conda env:', os.environ.get("CONDA_DEFAULT_ENV", ""))

	# Set game to online model.
	actors = [  "checkers_actor_white",
				"checkers_actor_black"]

	additional_to_load = {
		("eval_" + actor_name): (
			Evaluator.get_option_spec(name="eval_" + actor_name),
			lambda object_map, actor_name=actor_name: Evaluator(
				object_map, name="eval_" + actor_name,
				actor_name=actor_name, stats=None)
		)
		for i, actor_name in enumerate(actors)
	}
	additional_to_load.update({
		("mi_" + name): (ModelInterface.get_option_spec(), ModelInterface)
		for name in actors
	})



	# print("additional_to_load : ")

	# for i in additional_to_load:
	#     print(i, " : ", additional_to_load[i])

	# sys.exit(0)



	env = load_env(
		os.environ, num_models=2, overrides={'actor_only': True},
		additional_to_load=additional_to_load)

	GC = env["game"].initialize()

	stats = [Stats(), Stats()]

	for i in range(len(actors)):

		# "checkers_actor_white",
		# "checkers_actor_black"
		actor_name = actors[i]

		# Stats                 - статистика по выигрышам
		# e - Evaluator         - принимает в себя sampler и mi 
		#                   + вызывает actor(nn)
		# sampler - Sampler     - Used to sample an action from policy.
		# mi - ModelInterface   - хранит в себе модели и обновляет их по мере необходимости

		# print("env['sampler'] : ", env["sampler"])
		# print("env['mi_' + actor_name] : ", env["mi_" + actor_name])

		stat = stats[i]

		e = env["eval_" + actor_name]

		# e - Evaluator
		# ("eval_" + actor_name): (
		#     Evaluator.get_option_spec(name="eval_" + actor_name),
		#     lambda object_map, actor_name=actor_name: Evaluator(
		#         object_map, name="eval_" + actor_name,
		#         actor_name=actor_name, stats=None)
		# )

		# print("register actors env : ", env)
		# print("eval_" + actor_name, " = ", e)

		# print(f'register {actor_name} for e = {e!s}')
		e.setup(sampler=env["sampler"], mi=env["mi_" + actor_name])

		def actor(batch, e, stat):
			# print("HAHAHAHH")
			# print("batch : ",  batch["checkers_s"])

			reply = e.actor(batch)
			# print("actor reply : ", reply)
			# print("actor stat : ", stat)
			stat.feed(batch)
			return reply

		GC.reg_callback(actor_name,
						lambda batch, e=e, stat=stat: actor(batch, e, stat))

	root = os.environ.get("root", "./")
	# print(f'Root: "{root}"')
	args = env["game"].options
	loop_end = False

	def game_start(batch):
		print("In game start")
		print("batch['checkers_white_ver'] : ", batch["checkers_white_ver"])
		print("batch['checkers_black_ver'] : ", batch["checkers_black_ver"])

		vers = [
				int(batch["checkers_white_ver"][0]),
				int(batch["checkers_black_ver"][0])
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

	def game_end(batch):
		nonlocal loop_end
		wr = batch.GC.getClient().getCheckersGameStats().getWinRateStats()
		win_rate = (100.0 * wr.black_wins / (wr.black_wins + wr.white_wins)
					if (wr.black_wins + wr.white_wins) > 0 else 0.0)

		print("\x1b[1;31;40m|py|\x1b[0m", end="")
		print(f'[{datetime.now()!s}][selfplay.py=>main::game_end]', end="")
		print(f'\tB/W: {wr.black_wins}/{wr.white_wins}, ', end="")
		print(f'Both Lost: {wr.both_lost}, ', end="")
		print(f'Black winrate: {win_rate:.2f}, ', end="")
		print(f'Total Games:{wr.total_games}')

		if args.suicide_after_n_games > 0 and \
				wr.total_games >= args.suicide_after_n_games:
			print(f'\t#suicide_after_n_games: {args.suicide_after_n_games}, ')
			print(f'\ttotal_games: {wr.total_games}')
			loop_end = True

	GC.reg_callback_if_exists("game_start", game_start)
	GC.reg_callback_if_exists("game_end", game_end)

	GC.start()

	# print("model_loaders : ", env["model_loaders"])
	# print("GC.params : ", GC.params)
	
	if args.eval_model_pair:
		if args.eval_model_pair.find(",") >= 0:
			black, white = args.eval_model_pair.split(",")
		else:
			black = extract_ver(env["model_loaders"][0])
			white = extract_ver(env["model_loaders"][1])

			print("black : ", black)
			print("white : ", white)
			# Force them to reload in the future.
			for model_loader, actor_name in zip(env["model_loaders"], actors):
				reload_model(model_loader, GC.params,
							 env["mi_" + actor_name], actor_name, args)

		print("white : ", white)
		print("black : ", black)
		# We just use one thread to do selfplay.
		GC.GC.getClient().setRequest(
			int(black), int(white), 1)

	for actor_name in actors:
		env["eval_" + actor_name].episode_start(0)

	# sys.exit(0)

	while not loop_end:
		GC.run()

	GC.stop()


if __name__ == '__main__':
	main()
