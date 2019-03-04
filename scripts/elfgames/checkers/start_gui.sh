#!/usr/bin/env bash

MODEL=models/save-609280.bin
# shift

game=elfgames.checkers.game \
model=df_pred \
model_file=elfgames.checkers.df_model_checkers \
	python3 gui/CheckersServerELF.py \
	--server_addr localhost --port 1234 \
	\
	--batchsize 64 \
	--mode play			--keys_in_reply checkers_V checkers_rv \
	--load $MODEL \
	\
	--replace_prefix resnet.module,resnet init_conv.module,init_conv \
	--no_check_loaded_options \
	--no_parameter_print \
	\
	--verbose					--gpu 0 \
	--num_block 10		--dim 128 \
	\
	--mcts_virtual_loss 1 \
	--mcts_rollout_per_batch 1 \
	--mcts_persistent_tree \
	--use_mcts							--mcts_verbose_time \
	--mcts_use_prior				--mcts_puct 0.9 \
	--mcts_threads 1				--mcts_rollout_per_thread 100 \
	\
	# --dump_record_prefix mcts \

	#
	# "$@"

	# --replace_prefix resnet.module,resnet \
