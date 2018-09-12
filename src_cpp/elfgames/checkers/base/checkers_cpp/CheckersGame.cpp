#include "CheckersGame.hpp"

CheckersGame::CheckersGame() {
	this->history_n = 0;

	this->player_mapping = {
		{0, BLACK_PLAYER},
		{1, WHITE_PLAYER}
	};

	this->moves_to_index = all_moves::m_to_i;
	this->index_to_moves = all_moves::i_to_m;
	// this->states_history = None;

	// this->black_own_history_queue = None;
	// this->black_enemy_history_queue = None;
	// this->white_own_history_queue = None;
	// this->white_enemy_history_queue = None;
	this->board_impl = new CheckersBoard();

	this->reset();
}

// CheckersGame::CheckersGame(const GCheckersGameoState& s);

void				CheckersGame::reset() {
	this->board_impl->new_game();

	// this->states_history = {}
	// this->black_own_history_queue = deque([], maxlen=this->history_n)
	// this->black_enemy_history_queue = deque([], maxlen=this->history_n)
	// this->white_own_history_queue = deque([], maxlen=this->history_n)
	// this->white_enemy_history_queue = deque([], maxlen=this->history_n)

	// initial_state_black_own_history = this->board_impl.get_state_matrix_own(BLACK_PLAYER)
	// initial_state_black_enemy_history = this->board_impl.get_state_matrix_enemy(BLACK_PLAYER)
	// initial_state_white_own_history = this->board_impl.get_state_matrix_own(WHITE_PLAYER)
	// initial_state_white_enemy_history = this->board_impl.get_state_matrix_enemy(WHITE_PLAYER)

	// for(int i = 0; i < this->history_n; i++) {
	// 	this->black_own_history_queue.append(initial_state_black_own_history)
	// 	this->black_enemy_history_queue.append(initial_state_black_enemy_history)
	// 	this->white_own_history_queue.append(initial_state_white_own_history)
	// 	this->white_enemy_history_queue.append(initial_state_white_enemy_history)
	// }

	this->valid_moves[0] = this->get_valid_moves_new(0);
	this->valid_moves[1] = this->get_valid_moves_new(1);

	// this->has_repeated_states = false;
}


// def			CheckersGame::clone() {

// }

int					CheckersGame::get_cur_player() {
	return (this->board_impl->get_current_player());
}

int					CheckersGame::get_players_num() {
	return (TOTAL_PLAYERS);
}

int					CheckersGame::get_action_size() {
	return (ALL_ACTIONS);
}

std::vector<int>	CheckersGame::get_observation_size() {
	std::vector<int>	res;

	if (this->history_n != 0){
		res.push_back((this->history_n * 2));
	}
	res.push_back(BOARD_SIZE);
	res.push_back(BOARD_SIZE);
    return (res);
}

std::array<int, 2>		CheckersGame::make_move(int action_idx) {
	// action_idx - just index from 0 to 169(total siza of actions)
	std::array<int64_t, 2>		action;
	int							player;
	int							next_player;
	std::array<int, 2>			answer;


	player = this->get_cur_player();
	action = this->index_to_moves[action_idx];

	this->board_impl->make_move(action);

	next_player = this->get_cur_player();

	this->valid_moves[player] = this->get_valid_moves_new(player);
	this->valid_moves[next_player] = this->get_valid_moves_new(next_player);

	answer[0] = this->get_score(player);
	answer[1] = next_player;
	return (answer);
}

std::vector<int64_t>		CheckersGame::get_valid_moves(int player) {
	return (this->valid_moves[player]);
}

std::vector<int64_t>		CheckersGame::get_valid_moves_new(int player) {
	// vector <number, direction>
	std::vector<std::array<int64_t, 2>>	possible_moves;
	// binary vector of moves
	std::vector<int64_t> 	possible_idx_actions(ALL_ACTIONS);
	int 				inner_player;
	int					idx;

	inner_player = this->player_mapping[player];
	possible_moves = this->board_impl->get_legal_moves(inner_player);

	for (auto &action : possible_moves) {
		idx = this->moves_to_index[action];
		possible_idx_actions[idx] = 1;
	}

	return (possible_idx_actions);
}

bool			CheckersGame::is_ended() {
	std::vector<int64_t>	moves;
	int 					sum;

	moves = this->get_valid_moves_new(this->get_cur_player());
	sum = std::accumulate(moves.begin(), moves.end(), 0);
	
	return (this->is_draw() || sum == 0);
}

bool			CheckersGame::is_draw() {
	return (this->board_impl->is_draw());
}

int				CheckersGame::get_score(int player) {
	if (this->is_ended()) {
		if (this->is_draw())
			return (-1);
		if (this->get_valid_moves(player).size() == 0)
			return (-1);
		else
			return (1);
	}
	return (0);
}

// def			CheckersGame::get_observation(player) {

// }

// def			CheckersGame::get_observation_str(observation) {

// }


std::string		CheckersGame::get_display_str() {
// отображает оригинальное сотояние доски
	return (this->board_impl->get_true_state_str());
}





























// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG 
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG 
// DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG DEBUG 

// void   CheckersGame::get_moves_to_index(){
// 	for (auto it = this->moves_to_index.begin(); it != this->moves_to_index.end(); ++it) {
// 		std::cout << "[" << it->first[0] << ", ";
// 		std::cout << it->first[1] << "]";
// 		std::cout << " = " << it->second;
// 		std::cout << std::endl;
// 	}
// }

// void   CheckersGame::get_index_to_moves(){
// 	for (auto it = this->index_to_moves.begin(); it != this->index_to_moves.end(); ++it) {
// 		std::cout << it->first << " = ";
// 		std::cout << "[" << it->second[0] << ", ";
// 		std::cout << it->second[1] << "]";
// 		std::cout << std::endl;
// 	}
// }
