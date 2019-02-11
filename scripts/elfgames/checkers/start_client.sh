#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

echo $PYTHONPATH $SLURMD_NODENAME $CUDA_VISIBLE_DEVICES

root=./myserver \
game=elfgames.checkers.game \
model=df_pred model_file=elfgames.checkers.df_model_checkers \
	stdbuf -o 0 -e 0 python3 ./py/selfplay.py \
	\
	--server_id myserver			--port 1234 \
	--gpu 0 --gpu0 0 --gpu1 0\
	\
	--mode selfplay \
	--batchsize 512 \
	--num_games 16						--keys_in_reply checkers_V checkers_rv \
	--T 1 \
	--dim0 128								--dim1 128 \
	--num_block0 10						--num_block1 10 \
	\
	--use_mcts								--use_mcts_ai2 \
	--policy_distri_cutoff 5	--policy_distri_training_for_all \
	\
	--no_check_loaded_options2 \
	--no_check_loaded_options1 \
	--replace_prefix0 resnet.module,resnet init_conv.module,init_conv\
	--replace_prefix1 resnet.module,resnet init_conv.module,init_conv\
	--selfplay_timeout_usec 10 \
	\
	--use_fp160								--use_fp161 \
	--verbose \
	# --suicide_after_n_games 120
	

	# Params that client gets from server
	# MCTS:
	# --mcts_epsilon 0.25
	# --mcts_alpha 0.03
	# --mcts_puct 0.9
	# --mcts_threads 1
	# --mcts_rollout_per_thread 100
	# --mcts_virtual_loss 5
	# --mcts_persistent_tree
	# --mcts_use_prior