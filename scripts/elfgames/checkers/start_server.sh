#!/bin/bash

# Copyright (c) 2018-present, Facebook, Inc.
# All rights reserved.
#
# This source code is licensed under the BSD-style license found in the
# LICENSE file in the root directory of this source tree.



save=./myserver \
game=elfgames.checkers.game \
model=df_kl model_file=elfgames.checkers.df_model_checkers \
	stdbuf -o 0 -e 0 python3 -u ./py/train.py \
	\
	--server_id myserver		--port 1234 \
	--gpu 0 \
	\
	--mode train \
	--num_games 8					--keys_in_reply checkers_V \
	--T 1 \
	--dim 128 \
	--num_block 10 \
	\
	--batchsize 128 \
	--num_minibatch 1024 		--num_cooldown=128 \
	--bn_momentum=0					--momentum 0.9 \
	--weight_decay 0.0002		--opt_method sgd \
	--lr 0.01	\
	\
	--use_mcts							--use_mcts_ai2 \
	--mcts_epsilon 0.25			--mcts_alpha 0.03 \
	--mcts_puct 1.5					--mcts_use_prior \
	--mcts_threads 8				--mcts_rollout_per_thread 100 \
	--mcts_virtual_loss 1		--mcts_persistent_tree \
	\
	--save_first \
	\
	--use_data_parallel \
	\
	--num_episode 100000 \
	--keep_prev_selfplay \
	\
	--selfplay_async \
	--q_min_size 200				--q_max_size 2000		--num_reader 50 \
	\
	--selfplay_init_num 1000 \
	--selfplay_update_num 1000 \
	\
	--eval_winrate_thres 0.55 \
	--eval_num_games 200 \
	\
	--verbose \

	# 1>> server_log.log 2>&1 &
	# \
	
	# --tqdm \
	
	# --expected_num_client 10 \
	
	# --load myserver/save-15.bin \
	# 1>> log.log 2>&1 &
	

	# --batchsize 256			- прогон один раз через нейронку. берет этот батч и дает неронке для обучения.
	# --num_minibatch 128 - количество раз которое нужно давать нейронке --batchsize то есть 128 раз по 256 states

	# --selfplay_init_num 2	- проигрывает необходимое количество игр(для батча) 
	# после чего тренирует модель через
	#   |py| Trainer:: episode_start
	#   |py| Evaluator:: episode_start
	# 
	# Затем они ожидают пока заполниться CheckersGuardedRecords необходимое 
	# количество игр, чтобы отправить этот батч серверу на тренировку:
	#   q_min_size * num_reader; - необходимое количество игр
	# 
	# 
	# --selfplay_update_num 5 \
	# 
	# --num_episode 5				- количество обновлений модели на сервере, после чего сервер
	# 			посылает сигнал elf::base::Context-3 для остановки
	# 				
	# 
	# --eval_num_games 20		- после добавления модели ее можно сравнить с предидущей
	# 		eval_winrate_thres по этому параметру(выбирается лучшая)
	# 
	# 
	# --keep_prev_selfplay	- оставляет прошлые игры при апдейте модели и спользует их для
	# 			обучения













