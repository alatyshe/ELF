#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

MODEL=models/save-2048.bin

game=elfgames.russian_checkers.game \
model=df_pred \
model_file=elfgames.russian_checkers.model_russian_checkers \
	python3 ./py/play_console.py \
	--server_addr localhost --port 1234 \
	\
	--batchsize 64 \
	--mode play			--keys_in_reply V rv \
	--load $MODEL \
	\
	--replace_prefix resnet.module,resnet init_conv.module,init_conv \
	--no_check_loaded_options \
	--no_parameter_print \
	\
	--verbose					--gpu 0 \
	--num_block 10		--dim 128 \
	\
	--use_mcts \
	\
	--mcts_virtual_loss 0 \
	--mcts_rollout_per_batch 3 \
	--mcts_persistent_tree 	--mcts_verbose_time \
	--mcts_use_prior				--mcts_puct 0.9 \
	--mcts_threads 1				--mcts_rollout_per_thread 400 \
	\
	# "$@"
	# --black_use_policy_network_only \

	# --replace_prefix resnet.module,resnet \
