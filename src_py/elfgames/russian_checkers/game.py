#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

import inspect
import os

from elf import GCWrapper, ContextArgs, MoreLabels
from elf.options import auto_import_options, PyOptionSpec

import _elfgames_russian_checkers as russian_checkers
from server_addrs import addrs


class Loader(object):
	@classmethod
	def get_option_spec(cls):
		spec = PyOptionSpec()
		spec.addBoolOption(
			'actor_only',
			'TODO: fill this help message in',
			False)
		spec.addStrListOption(
			'list_files',
			'Provide a list of json files for offline training',
			[])
		spec.addIntOption(
			'port',
			'TODO: fill this help message in',
			5556)
		spec.addStrOption(
			'server_addr',
			'TODO: fill this help message in',
			'')
		spec.addStrOption(
			'server_id',
			'TODO: fill this help message in',
			'')
		spec.addIntOption(
			'q_min_size',
			'TODO: fill this help message in',
			10)
		spec.addIntOption(
			'q_max_size',
			'TODO: fill this help message in',
			1000)
		spec.addIntOption(
			'num_reader',
			'TODO: fill this help message in',
			50)
		spec.addIntOption(
			'num_reset_ranking',
			'TODO: fill this help message in',
			5000)
		spec.addIntOption(
			'client_max_delay_sec',
			'Maximum amount of allowed delays in sec. If the client '
			'didn\'t respond after that, we think it is dead.',
			1200)
		spec.addBoolOption(
			'verbose',
			'TODO: fill this help message in',
			False)
		spec.addBoolOption(
			'keep_prev_selfplay',
			'TODO: fill this help message in',
			False)
		spec.addIntOption(
			'num_games_per_thread',
			('For offline mode, it is the number of concurrent games per '
			 'thread, used to increase diversity of games; for selfplay mode, '
			 'it is the number of games played at each thread, and after that '
			 'we need to call restartAllGames() to resume.'),
			-1)
		spec.addIntOption(
			'expected_num_clients',
			'Expected number of clients',
			-1
		)
		spec.addIntOption(
			'checkers_num_future_actions',
			'TODO: fill this help message in',
			1)
		spec.addStrOption(
			'mode',
			'TODO: fill this help message in',
			'play')
		spec.addBoolOption(
			'black_use_policy_network_only',
			'TODO: fill this help message in',
			False)
		spec.addBoolOption(
			'white_use_policy_network_only',
			'TODO: fill this help message in',
			False)
		spec.addBoolOption(
			'use_mcts',
			'TODO: fill this help message in',
			False)
		spec.addBoolOption(
			'use_mcts_ai2',
			'TODO: fill this help message in',
			False)
		spec.addFloatOption(
			'white_puct',
			'PUCT for white when it is > 0.0. If it is -1 then we use'
			'the same puct for both side (specified by mcts_options).'
			'A HACK to use different puct for different model. Should'
			'be replaced by a more systematic approach.',
			-1.0)
		spec.addIntOption(
			'white_mcts_rollout_per_batch',
			'white mcts rollout per batch',
			-1)
		spec.addIntOption(
			'white_mcts_rollout_per_thread',
			'white mcts rollout per thread',
			-1)
		spec.addStrOption(
			'dump_record_prefix',
			'TODO: fill this help message in',
			'')
		spec.addIntOption(
			'policy_distri_cutoff',
			'first N moves will be randomly',
			0)
		spec.addIntOption(
			'selfplay_timeout_usec',
			'TODO: fill this help message in',
			0)
		spec.addIntOption(
			'gpu',
			'TODO: fill this help message in',
			-1)
		spec.addBoolOption(
			'policy_distri_training_for_all',
			'TODO: fill this help message in',
			False)
		spec.addBoolOption(
			'parameter_print',
			'TODO: fill this help message in',
			True)
		spec.addIntOption(
			'batchsize',
			'batch size',
			128)
		spec.addIntOption(
			'batchsize2',
			'batch size',
			-1)
		spec.addIntOption(
			'T',
			'number of timesteps',
			6)
		spec.addIntOption(
			'selfplay_init_num',
			('Initial number of selfplay games to generate before training a '
			 'new model'),
			2000)
		spec.addIntOption(
			'selfplay_update_num',
			('Additional number of selfplay games to generate after a model '
			 'is updated'),
			1000)
		spec.addBoolOption(
			'selfplay_async',
			('Whether to use async mode in selfplay'),
			False)
		spec.addIntOption(
			'eval_num_games',
			('number of evaluation to be performed to decide whether a model '
			 'is better than the other'),
			400)
		spec.addFloatOption(
			'eval_winrate_thres',
			'Win rate threshold for evalution',
			0.55)
		spec.addIntOption(
			'eval_old_model',
			('If specified, then we directly switch to evaluation mode '
			 'between the loaded model and the old model specified by this '
			 'switch'),
			-1)
		spec.addStrOption(
			'eval_model_pair',
			('If specified for df_selfplay.py, then the two models will be '
			 'evaluated on this client'),
			'')
		spec.addBoolOption(
			'cheat_eval_new_model_wins_half',
			'When enabled, in evaluation mode, when the game '
			'finishes, the player with the most recent model gets 100% '
			'win rate half of the time.'
			'This is used to test the framework',
			False)
		spec.addBoolOption(
			'cheat_selfplay_random_result',
			'When enabled, in selfplay mode the result of the game is random'
			'This is used to test the framework',
			False)
		spec.addBoolOption(
			'human_plays_for_black',
			'',
			False)
		spec.addIntOption(
			'suicide_after_n_games',
			'return after n games have finished, -1 means it never ends',
			-1)

		spec.merge(PyOptionSpec.fromClasses((ContextArgs, MoreLabels)))
		return spec


	@auto_import_options
	def __init__(self, option_map):
		self.context_args = ContextArgs(option_map)
		self.more_labels = MoreLabels(option_map)


	def _set_params(self):
		co = russian_checkers.ContextOptions()
		self.context_args.initialize(co)
		co.job_id = os.environ.get("job_id", "local")
		if self.options.parameter_print:
			co.print()

		game_opt = russian_checkers.CheckersGameOptions()

		game_opt.seed = 0
		game_opt.list_files = self.options.list_files

		if self.options.server_addr:
			game_opt.server_addr = self.options.server_addr
		else:
			if self.options.server_id:
				game_opt.server_addr = addrs[self.options.server_id]
				game_opt.server_id = self.options.server_id
			else:
				game_opt.server_addr = ""
				game_opt.server_id = ""

		game_opt.port = self.options.port
		game_opt.mode = self.options.mode
		game_opt.use_mcts = self.options.use_mcts
		game_opt.use_mcts_ai2 = self.options.use_mcts_ai2
		game_opt.dump_record_prefix = self.options.dump_record_prefix
		game_opt.policy_distri_training_for_all = \
			self.options.policy_distri_training_for_all
		game_opt.verbose = self.options.verbose
		game_opt.black_use_policy_network_only = \
			self.options.black_use_policy_network_only
		game_opt.white_use_policy_network_only = \
			self.options.white_use_policy_network_only
		game_opt.q_min_size = self.options.q_min_size
		game_opt.q_max_size = self.options.q_max_size
		game_opt.num_reader = self.options.num_reader
		game_opt.checkers_num_future_actions = self.options.checkers_num_future_actions
		game_opt.num_reset_ranking = self.options.num_reset_ranking
		game_opt.policy_distri_cutoff = self.options.policy_distri_cutoff
		game_opt.num_games_per_thread = self.options.num_games_per_thread
		game_opt.keep_prev_selfplay = self.options.keep_prev_selfplay
		game_opt.expected_num_clients = self.options.expected_num_clients

		game_opt.white_puct = self.options.white_puct
		game_opt.white_mcts_rollout_per_batch = \
			self.options.white_mcts_rollout_per_batch
		game_opt.white_mcts_rollout_per_thread = \
			self.options.white_mcts_rollout_per_thread

		game_opt.client_max_delay_sec = self.options.client_max_delay_sec
		game_opt.selfplay_init_num = self.options.selfplay_init_num
		game_opt.selfplay_update_num = self.options.selfplay_update_num
		game_opt.selfplay_async = self.options.selfplay_async
		game_opt.eval_num_games = self.options.eval_num_games
		game_opt.eval_thres = self.options.eval_winrate_thres
		game_opt.cheat_eval_new_model_wins_half = \
			self.options.cheat_eval_new_model_wins_half
		game_opt.cheat_selfplay_random_result = \
			self.options.cheat_selfplay_random_result

		if self.options.human_plays_for_black:
			game_opt.human_plays_for = 0
		else:
			game_opt.human_plays_for = 1

		self.max_batchsize = max(
			self.options.batchsize, self.options.batchsize2) \
			if self.options.batchsize2 > 0 \
			else self.options.batchsize
		co.batchsize = self.max_batchsize

		GC = russian_checkers.GameContext(co, game_opt)

		if self.options.parameter_print:
			print("************ CheckersGameOptions ************")
			print(game_opt.info())
			print("*********************************************")
			print("Version: ", GC.ctx().version())
			print("*********************************************")

		return co, GC, game_opt


	def initialize(self):
		co, GC, game_opt = self._set_params()

		params = GC.getParams()

		if self.options.parameter_print:
			print("Mode: ", game_opt.mode)
			print("checkers_num_action: ", params["checkers_num_action"])

		desc = {}

		if self.options.mode == "play":
			desc["human_actor"] = dict(
				input=["checkers_s"],
				reply=["pi", 
						"a", 
						"V"],
				batchsize=1,
			)
			desc["checkers_actor_black"] = dict(
				input=["checkers_s"],
				reply=["pi", 
						"V", 
						"a", 
						"rv"],
				timeout_usec=10,
				batchsize=co.mcts_options.num_rollouts_per_batch
			)

		elif self.options.mode == "selfplay":
			desc["game_end"] = dict(
				batchsize=1,
			)
			desc["game_start"] = dict(
				batchsize=1,
				input=["checkers_white_ver", 
						"checkers_black_ver"],
				reply=None
			)
			# checkers
			desc["checkers_actor_white"] = dict(
				input=["checkers_s"],
				reply=["pi", 
						"V", 
						"a", 
						"rv"],
				batchsize=self.options.batchsize2
				if self.options.batchsize2 > 0
				else self.options.batchsize,
				timeout_usec=self.options.selfplay_timeout_usec,
			)            
			desc["checkers_actor_black"] = dict(
				input=["checkers_s"],
				reply=["pi", 
						"V", 
						"a", 
						"rv"],
				batchsize=self.options.batchsize2
				if self.options.batchsize2 > 0
				else self.options.batchsize,
				timeout_usec=self.options.selfplay_timeout_usec,
			)

		elif self.options.mode == "train" or self.options.mode == "offline_train":
			desc["train"] = dict(
				input=["checkers_s", 
						"checkers_offline_a", 
						"checkers_winner", 
						"checkers_mcts_scores", 
						"checkers_move_idx",
						"checkers_selfplay_ver"],
				reply=None
			)
			desc["train_ctrl"] = dict(
				input=["checkers_selfplay_ver"],
				reply=None,
				batchsize=1
			)

		else:
			raise "No such mode: " + self.options.mode

		params.update(dict(
			num_group=1 if self.options.actor_only else 2,
			T=self.options.T,
		))

		self.more_labels.add_labels(desc)
		return GCWrapper(
			GC,
			self.max_batchsize,
			desc,
			num_recv=2,
			gpu=(self.options.gpu
				 if (self.options.gpu is not None and self.options.gpu >= 0)
				 else None),
			use_numpy=False,
			params=params,
			verbose=self.options.parameter_print)




