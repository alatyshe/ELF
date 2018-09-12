#pragma once

# include <numeric>
# include "CheckersBoard.hpp"
# include "hash_all_moves.hpp"

// # define CHECK_REPEATED_STATES false
# define BLACK_PLAYER 	0 
# define WHITE_PLAYER 	1
# define ALL_ACTIONS 	170
# define BOARD_SIZE		8
# define TOTAL_PLAYERS	2


class CheckersGame {

public:
	CheckersGame();
	// CheckersGame::CheckersGame(const GCheckersGameoState& s);
	~CheckersGame() {};

	void					reset();
	// def			clone();
	int						get_cur_player();
	int						get_players_num();
	int						get_action_size();
	std::vector<int>		get_observation_size();
	std::array<int, 2>		make_move(int action_idx);
	std::vector<int64_t>	get_valid_moves(int player);
	
	bool					is_ended();
	bool					is_draw();
	int						get_score(int player);

	std::string				get_display_str();

	// def			get_observation(int player);
	// def			get_observation_str(observation);


private:
	int					history_n;
	std::map<int, int>	player_mapping;
	CheckersBoard		*board_impl;

	std::map<std::array<int64_t, 2>, int>	moves_to_index;
	std::map<int, std::array<int64_t, 2>>	index_to_moves;

	// ??????
	std::array<std::vector<int64_t>, 2>		valid_moves;
	// black_own_history_queue = None
	// black_enemy_history_queue = None
	// white_own_history_queue = None
	// white_enemy_history_queue = None	

	std::vector<int64_t>	get_valid_moves_new(int player);
};