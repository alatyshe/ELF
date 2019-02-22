#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

echo $PYTHONPATH $SLURMD_NODENAME $CUDA_VISIBLE_DEVICES

root=./models \
game=elfgames.checkers.game \
model=df_pred model_file=elfgames.checkers.df_model_checkers \
	stdbuf -o 0 -e 0 python3 ./py/selfplay.py \
	\
	--server_id myserver			--port 1234 \
	--gpu 0 --gpu0 0 --gpu1 0\
	\
	--mode selfplay \
	--batchsize 64 \
	--num_games 4							--keys_in_reply checkers_V checkers_rv \
	--T 1 \
	--dim0 128								--dim1 128 \
	--num_block0 10						--num_block1 10 \
	\
	--use_mcts								--use_mcts_ai2 \
	--policy_distri_cutoff 5	--policy_distri_training_for_all \
	\
	--no_check_loaded_options0 \
	--no_check_loaded_options1 \
	--replace_prefix0 resnet.module,resnet init_conv.module,init_conv\
	--replace_prefix1 resnet.module,resnet init_conv.module,init_conv\
	--selfplay_timeout_usec 10 \
	\
	--use_fp160								--use_fp161 \
	--verbose \
	
	# --dump_record_prefix tree\
	
	# --suicide_after_n_games 120


	# --num_games - int, 'number of games'
	# --batchsize - int, 'batch size'
	# --T - int, 'number of timesteps'
	
	# Params that client gets from server
	# 	TSOptions
	# --mcts_threads - int 'number of MCTS threads'
	# --mcts_rollout_per_thread - int, 'number of rollotus per MCTS thread'
	# --mcts_rollout_per_batch - int, 'Batch size for mcts rollout'
	# --mcts_verbose - bool, 'enables mcts verbosity'
	# --mcts_verbose_time - bool, 'enables mcts verbosity for time stats'
	# --mcts_persistent_tree  - bool, 'use persistent tree in MCTS'
	# --mcts_epsilon - float, 'for exploration enhancement, weight of randomization'
	# --mcts_alpha - float, 'for exploration enhancement, alpha term in gamma distribution'	
	# --mcts_pick_method - string, 'criterion for mcts node selection' 'most_visited'
	# --mcts_virtual_loss - int, '"virtual" number of losses for MCTS edges'

	# 	SearchAlgoOptions
	# --mcts_use_prior - bool, 'use prior in MCTS'
	# --mcts_puct - float, 'prior weight'
	# --mcts_unexplored_q_zero - bool, 'set all unexplored node to have Q value zero'
	# --mcts_root_unexplored_q_zero - bool, 'set unexplored child of root node to have Q value zero'
	
	
	
	