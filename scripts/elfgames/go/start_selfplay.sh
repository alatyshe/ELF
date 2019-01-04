#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

LOAD0=myserver/save-0.bin
LOAD1=myserver/save-128.bin

BATCHSIZE=128
NUM_ROLLOUTS=100

BATCHSIZE2=128
NUM_ROLLOUTS2=100

GPU=0

DIM=128
NUM_BLOCK=10

game=elfgames.go.game \
model=df_pred \
model_file=elfgames.go.df_model_checkers python3 selfplay.py \
	\
	--server_id myserver		--port 2341 \
	--mode selfplay \
	--num_games 4 \
	--keys_in_reply checkers_V checkers_rv\
	\
	--batchsize $BATCHSIZE		--mcts_rollout_per_batch $BATCHSIZE \
	--mcts_threads 16			--mcts_rollout_per_thread $NUM_ROLLOUTS \
	--mcts_virtual_loss 1		--mcts_epsilon 0.0 \
	--mcts_alpha 0.00 \
	--use_mcts					--use_mcts_ai2 \
	--mcts_use_prior \
	--mcts_persistent_tree		--mcts_puct 0.85 \
	\
	--batchsize2 $BATCHSIZE2	--white_mcts_rollout_per_batch $BATCHSIZE2 \
	--white_mcts_rollout_per_thread $NUM_ROLLOUTS2 \
	\
	--eval_model_pair loaded \
	--policy_distri_cutoff 0 \
	--num_block0 $NUM_BLOCK		--dim0 $DIM \
	--num_block1 $NUM_BLOCK		--dim1 $DIM \
	--no_check_loaded_options0 \
	--no_check_loaded_options1 \
	--verbose \
	--gpu $GPU \
	--load0 $LOAD0 \
	--load1 $LOAD1 \
	--use_fp160					--use_fp161 \
	--gpu $GPU \
	--replace_prefix0 resnet.module,resnet init_conv.module,init_conv\
	--replace_prefix1 resnet.module,resnet init_conv.module,init_conv\
	--selfplay_timeout_usec 10 \
	\
	--suicide_after_n_games 100 \
	"$@"

# --resign_thres 0.05 \