#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.



save=./myserver \
game=elfgames.go.game \
model=df_kl model_file=elfgames.go.df_model_checkers \
    stdbuf -o 0 -e 0 python -u ./train.py \
    \
    --server_id myserver        --port 1234 \
    \
    --mode train \
    --batchsize 2048            --num_minibatch 1000\
    --num_games 2048            --keys_in_reply checkers_V \
    --T 1 \
    --dim 224                   --gpu 0 \
    --num_block 5 \
    \
    --use_mcts                  --use_mcts_ai2 \
    --mcts_epsilon 0.25         --mcts_persistent_tree \
    --mcts_puct 0.85            --mcts_rollout_per_thread 100 \
    --mcts_threads 16           --mcts_use_prior \
    --mcts_virtual_loss 5       --mcts_alpha 0.03 \
    \
    --eval_winrate_thres 0.55 \
    --save_first \
    \
    --use_data_parallel \
    \
    --num_episode 1000000 \
    --keep_prev_selfplay        --keep_prev_selfplay \
    \
    --weight_decay 0.0002       --opt_method sgd \
    --bn_momentum=0             --num_cooldown=50 \
    --expected_num_client 10 \
    \
    --selfplay_async \
    --selfplay_init_num 20 \
    --q_min_size 100            --q_max_size 4000       --num_reader 2 \
    --selfplay_update_num 100 \
    --eval_num_games 100 \
    \
    --lr 0.01                   --momentum 0.9 \
    # 1>> log.log 2>&1 &

    # --selfplay_init_num 2 - после него начинает свою работу
    #   |py| Trainer:: episode_start
    #   |py| Evaluator:: episode_start
    # и начинает собирать батчи
    # 
    # 
    # Затем они ожидают пока заполниться CheckersGuardedRecords необходимое 
    # количество игр, чтобы отправить этот батч серверу на тренировку:
    #   q_min_size * num_reader; - необходимое количество игр
    # 
    # 
    # --selfplay_update_num 5 \
    # 
    # 
    # --eval_num_games 2 \