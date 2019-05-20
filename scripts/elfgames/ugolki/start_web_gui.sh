#!/usr/bin/env bash


# MODEL=models/save-4096.bin
# MODEL=models/save-14336.bin
# MODEL=models/save-43008.bin
# MODEL=models/save-47104.bin
MODEL=new_models/save-10240.bin
# shift

game=elfgames.checkers.game \
model=df_pred \
model_file=elfgames.checkers.df_model_checkers \
	python3 web_gui/play_gui.py \
	--server_addr localhost --port 1234 \
	\
	--batchsize 512 \
	--mode play			--keys_in_reply V rv \
	--load $MODEL \
	\
	--replace_prefix resnet.module,resnet init_conv.module,init_conv \
	--no_check_loaded_options \
	--no_parameter_print \
	\
	--verbose					--gpu 0 	\
	--num_block 10		--dim 256 \
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
