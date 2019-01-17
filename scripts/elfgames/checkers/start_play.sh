#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

MODEL=myserver/save-128.bin
# shift

game=elfgames.checkers.game \
model=df_pred \
model_file=elfgames.checkers.df_model_checkers python3 ./py/play_console.py \
	--server_addr localhost --port 1234 \
	\
	--mode online --keys_in_reply checkers_V checkers_rv \
	--use_mcts --mcts_verbose_time \
	--mcts_use_prior --mcts_persistent_tree --load $MODEL \
	\
	--replace_prefix resnet.module,resnet init_conv.module,init_conv \
	--no_check_loaded_options \
	--no_parameter_print \
	\
	--verbose --gpu 0 \
	--num_block 10 --dim 128 \
	\
	--mcts_puct 1.50 --batchsize 16 --mcts_rollout_per_batch 16 \
	--mcts_threads 16 --mcts_rollout_per_thread 100 --mcts_virtual_loss 1
	# "$@"

	# --replace_prefix resnet.module,resnet \
