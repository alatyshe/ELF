#!/usr/bin/env bash

# MODEL=models/save-4096.bin
# MODEL=models/save-14336.bin
# MODEL=models/save-43008.bin
# MODEL=models/save-47104.bin
MODEL=models/save-0.bin
# shift

game=elfgames.russian_checkers.game \
model=df_pred \
model_file=elfgames.russian_checkers.model_russian_checkers \
	python3 web_gui/play_gui.py \
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
	--mcts_rollout_per_batch 1 \
	--use_mcts							--mcts_verbose_time \
	--mcts_puct 1.5 				--mcts_use_prior \
	--mcts_threads 8				--mcts_rollout_per_thread 100 \
	--mcts_virtual_loss 1 	--mcts_persistent_tree \
	\
	# --human_plays_for_black
	# \
	# --dump_record_prefix mcts \

	#
	# "$@"

	# --replace_prefix resnet.module,resnet \
