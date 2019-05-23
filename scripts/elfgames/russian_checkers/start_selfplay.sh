#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

LOAD1=models/save-609280.bin
LOAD0=models/save-609280.bin

BATCHSIZE=5
NUM_ROLLOUTS=50

BATCHSIZE2=5
NUM_ROLLOUTS2=50

GPU=0

DIM=128
NUM_BLOCK=10

game=elfgames.russian_checkers.game \
model=df_pred \
model_file=elfgames.russian_checkers.model_russian_checkers \
	python3 ./py/selfplay.py \
	\
	--T 1 \
	--server_id myserver		--port 2341 \
	--gpu $GPU --gpu0 $GPU  --gpu1 $GPU\
	\
	--mode selfplay \
	--num_games 1 \
	--keys_in_reply V rv\
	\
	--batchsize $BATCHSIZE \
	--mcts_rollout_per_batch $BATCHSIZE \
	--mcts_rollout_per_thread $NUM_ROLLOUTS \
	\
	--batchsize2 $BATCHSIZE2 \
	--white_mcts_rollout_per_batch $BATCHSIZE2 \
	--white_mcts_rollout_per_thread $NUM_ROLLOUTS2 \
	\
	--use_mcts							--use_mcts_ai2 \
	--mcts_virtual_loss 3		--mcts_epsilon 0.0 \
	--mcts_alpha 0.00 			--mcts_threads 8\
	--mcts_use_prior \
	--mcts_persistent_tree	--mcts_puct 0.9 \
	\
	--eval_model_pair loaded \
	--policy_distri_cutoff 5 \
	--num_block0 $NUM_BLOCK		--dim0 $DIM \
	--num_block1 $NUM_BLOCK		--dim1 $DIM \
	--no_check_loaded_options0 \
	--no_check_loaded_options1 \
	--verbose \
	--load0 $LOAD0 \
	--load1 $LOAD1 \
	--use_fp160					--use_fp161 \
	--replace_prefix0 resnet.module,resnet init_conv.module,init_conv\
	--replace_prefix1 resnet.module,resnet init_conv.module,init_conv\
	--selfplay_timeout_usec 10 \
	\
	--suicide_after_n_games 100 \
	"$@"

# --resign_thres 0.05 \


