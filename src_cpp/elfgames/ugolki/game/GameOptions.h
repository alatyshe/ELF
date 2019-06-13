#pragma once

#include <sstream>
#include <string>

// elf
#include "elf/legacy/pybind_helper.h"
#include "elf/utils/utils.h"

// Options filled in game.py
struct GameOptions {
  // Seed.
  unsigned int seed;
  int num_future_actions = 0;

  int human_plays_for = 0;

  // For offline training,
  //    This means how many games to open simultaneously per thread?
  //    When sending current situations, randomly select one to break any
  //    correlations.
  // For selfplay setting.
  //    This means how many games are played sequentially in each thread.
  int num_games_per_thread = -1;

  // mode == "play": In this mode, the thread will 
  //    not output the next k moves (since every game is new).
  //    Instead, it will get the action from the neural network to proceed.
  // mode == "offline": offline training
  // mode == "selfplay": self play.
  std::string mode;

  // Use mcts engine.
  bool use_mcts = false;
  bool use_mcts_ai2 = false;

  // Specify which side uses policy network only.
  bool black_use_policy_network_only = false;
  bool white_use_policy_network_only = false;

  // Cutoff ply for mcts policy / best a
  int policy_distri_cutoff = 20;
  bool policy_distri_training_for_all = false;

  int num_reset_ranking = 5000;

  int q_min_size = 10;
  int q_max_size = 1000;
  int num_reader = 50;

  // Second puct used for ai2, if -1 then use the same puct.
  float       white_puct = -1.0;
  int white_mcts_rollout_per_batch = -1;
  int white_mcts_rollout_per_thread = -1;

  int eval_num_games = 400;
  float eval_thres = 0.55;
  int eval_num_threads = 4;

  // Default it is 20 min. During intergration test we could make it shorter.
  int client_max_delay_sec = 1200;

  // Initial number of selfplay games for each model used for selfplay.
  int selfplay_init_num = 5000;
  // Additive number of selfplay after the new model is updated.
  int selfplay_update_num = 1000;

  // Whether we use async mode for selfplay.
  bool selfplay_async = false;

  bool cheat_eval_new_model_wins_half = false;
  bool cheat_selfplay_random_result = false;

  bool keep_prev_selfplay = false;

  
  int expected_num_clients = -1;

  // A list file containing the files to load.
  std::vector<std::string> list_files;
  std::string server_addr;
  std::string server_id;
  int port;
  
  bool verbose = false;
  std::string dump_record_prefix;

  std::string selfplay_records_directory;
  std::string eval_records_directory;
  std::string records_buffer_directory;

  std::string time_signature;

  GameOptions() {
    time_signature = elf_utils::time_signature();
  }

  std::string info() const {
    std::stringstream ss;

    ss << std::setw(30) << std::right;
    ss << "Seed: " << seed << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Time signature: " << time_signature << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Client max delay in sec: " << client_max_delay_sec << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Num future actions: " << num_future_actions << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Games per thread: " << num_games_per_thread << std::endl;
    ss << std::setw(30) << std::right;
    ss << "mode: " << mode << std::endl;

    ss << std::setw(30) << std::right;
    ss << "Keep prev Selfplay: " << keep_prev_selfplay << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Init min games: " << selfplay_init_num << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Update games: " << selfplay_update_num << std::endl;
    
    ss << std::setw(30) << std::right;
    ss << "Eval num games: " << eval_num_games << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Eval Threshold: " << eval_thres << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Eval num Threads: " << eval_num_threads << std::endl;

    ss << std::setw(30) << std::right;
    ss << "Async: " << elf_utils::print_bool(selfplay_async) << std::endl;

    ss << std::setw(30) << std::right;
    ss << "UseMCTS: " << elf_utils::print_bool(use_mcts) << std::endl;
    ss << std::setw(30) << std::right;
    ss << "UseMCTS AI2: " << elf_utils::print_bool(use_mcts_ai2) << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Black policy network only: " << elf_utils::print_bool(black_use_policy_network_only) << std::endl;
    ss << std::setw(30) << std::right;
    ss << "White policy network only: " << elf_utils::print_bool(white_use_policy_network_only) << std::endl;

    ss << std::setw(30) << std::right;
    ss << "PolicyDistriCutOff: " << policy_distri_cutoff << std::endl;

    if (expected_num_clients > 0) {
      ss << std::setw(30) << std::right;
      ss << "Expected clients: " << expected_num_clients << std::endl;
    }

    if (!list_files.empty()) {
      ss << "ListFile[" << list_files.size() << "]: ";
      for (const std::string& f : list_files) {
        ss << f << ", ";
      }
      ss << std::endl;
    }

    ss << std::setw(30) << std::right;
    ss << std::endl
       << "Server options: "
       << server_addr
       << "[server_id=" << server_id << "]"
       << "[port=" << port << "]"
       << std::endl;

    ss << std::setw(30) << std::right;
    ss << "Num Reader: " << num_reader << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Q_min_size: " << q_min_size << std::endl;
    ss << std::setw(30) << std::right;
    ss << "Q_max_size: " << q_max_size << std::endl;

    ss << std::setw(30) << std::right;
    ss << "Verbose: " << elf_utils::print_bool(verbose) << std::endl;

    ss << "Policy distri training for all moves: "
       << elf_utils::print_bool(policy_distri_training_for_all) << std::endl;

    if (!dump_record_prefix.empty()) {
      ss << std::setw(30) << std::right;
      ss << "dumpRecord: " << dump_record_prefix << std::endl;
    }

    if (!eval_records_directory.empty()) {
      ss << std::setw(60) << std::right;
      ss << "Eval Records dump directory : " << eval_records_directory << std::endl;      
    }

    if (!selfplay_records_directory.empty()) {
      ss << std::setw(60) << std::right;
      ss << "SelfPlay Records dump directory : " << selfplay_records_directory << std::endl;
    }

    if (!records_buffer_directory.empty()) {
      ss << std::setw(60) << std::right;
      ss << "Simple buffer Records dump directory : " << records_buffer_directory << std::endl;
    }

    ss << std::setw(30) << std::right;
    ss << "Reset move ranking after: " << num_reset_ranking << " actions"
       << std::endl;

    if (white_puct > 0.0) {
      ss << std::setw(30) << std::right;
      ss << "White puct: " << white_puct << std::endl;
      ss << std::setw(30) << std::right;
      ss << "White MCTS rollout per batch: " << white_mcts_rollout_per_batch << std::endl;
      ss << std::setw(30) << std::right;
      ss << "White MCTS rollout per thread: " << white_mcts_rollout_per_thread << std::endl;
    }

    ss << std::setw(30) << std::right;
    ss << "Client max delay in sec: " << client_max_delay_sec << std::endl;

    if (cheat_eval_new_model_wins_half)
      ss << "Cheat mode: New model gets 100% win rate half of the time."
         << std::endl;

    if (cheat_selfplay_random_result)
      ss << "Cheat selfplay mode: Random outcome." << std::endl;

    return ss.str();
  }

  REGISTER_PYBIND_FIELDS(
      seed,
      mode,
      num_future_actions,
      list_files,
      verbose,
      num_games_per_thread,
      use_mcts,
      server_addr,
      server_id,
      port,
      policy_distri_cutoff,
      client_max_delay_sec,
      q_min_size,
      q_max_size,
      num_reader,
      dump_record_prefix,
      selfplay_records_directory,
      eval_records_directory,
      records_buffer_directory,
      use_mcts_ai2,
      num_reset_ranking,
      policy_distri_training_for_all,
      black_use_policy_network_only,
      white_use_policy_network_only,
      cheat_eval_new_model_wins_half,
      cheat_selfplay_random_result,
      eval_num_games,
      selfplay_init_num,
      selfplay_update_num,
      selfplay_async,
      white_puct,
      white_mcts_rollout_per_batch,
      white_mcts_rollout_per_thread,
      eval_thres,
      keep_prev_selfplay,
      expected_num_clients,
      human_plays_for);
};
