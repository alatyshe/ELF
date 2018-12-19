#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.

echo $PYTHONPATH $SLURMD_NODENAME $CUDA_VISIBLE_DEVICES

root=./myserver \
game=elfgames.go.game \
model=df_pred model_file=elfgames.go.df_model_checkers \
    stdbuf -o 0 -e 0 python ./selfplay.py \
    \
    --server_id myserver        --port 1234 \
    \
    --mode selfplay \
    --batchsize 64 \
    --num_games 1               --keys_in_reply checkers_V checkers_rv \
    --T 1 \
    --dim0 64                   --dim1 64       --gpu 0 \
    --num_block0 5              --num_block1 5 \
    \
    --use_mcts                  --use_mcts_ai2 \
    --mcts_epsilon 0.25         --mcts_persistent_tree \
    --mcts_puct 0.85            --mcts_rollout_per_thread 16 \
    --mcts_threads 16           --mcts_use_prior \
    --mcts_virtual_loss 5       --mcts_alpha 0.03 \
    \
    --suicide_after_n_games 200 \
    --policy_distri_cutoff 30   --policy_distri_training_for_all \
    \
    --no_check_loaded_options0  --no_check_loaded_options1 \
    --replace_prefix0 resnet.module,resnet init_conv.module,init_conv\
    --replace_prefix1 resnet.module,resnet init_conv.module,init_conv\
    --selfplay_timeout_usec 10 \
    \
    --use_fp160 --use_fp161 \
    --verbose 
    # \
    # 1>> client.log 2>&1 &
    # --parameter_print