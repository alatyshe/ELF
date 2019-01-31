#include "CheckersStateExt.h"

CheckersStateExt::CheckersStateExt(int game_idx, const CheckersGameOptions& options)
		: _game_idx(game_idx),
			_last_move_for_the_game(M_INVALID),
			_last_value(0.0),
			_options(options),
			_logger(
					elf::logging::getIndexedLogger(
						MAGENTA_B + std::string("|++|") + COLOR_END + 
						"CheckersStateExt-", 
						"")) {
	restart();
}


void CheckersStateExt::setRequest(const MsgRequest& request) {
	_curr_request = request;
}

void									CheckersStateExt::addCurrentModel() {
	if (_curr_request.vers.black_ver >= 0)
		_using_models.insert(_curr_request.vers.black_ver);
	if (_curr_request.vers.white_ver >= 0)
		_using_models.insert(_curr_request.vers.white_ver);
}

const MsgRequest& CheckersStateExt::currRequest() const {
	return _curr_request;
}

Coord CheckersStateExt::lastMove() const {
	if (_state.justStarted())
		return _last_move_for_the_game;
	else
		return _state.lastMove();
}

void CheckersStateExt::restart() {
	_last_value = _state.getFinalValue();
	_state.reset();
	_mcts_policies.clear();
	_predicted_values.clear();
	_using_models.clear();
	_seq++;

	addCurrentModel();
}

ThreadState CheckersStateExt::getThreadState() const {
	ThreadState s;

	s.thread_id = _game_idx;
	s.seq = _seq;
	s.move_idx = _state.getPly() - 1;
	s.black = _curr_request.vers.black_ver;
	s.white = _curr_request.vers.white_ver;
	return s;
}

// Reward за последнюю сыгранную игру.
float CheckersStateExt::getLastGameFinalValue() const {
	return _last_value;
}

// Передаем индекс нашего шага и делаем шаг вперед для нашей доски
bool CheckersStateExt::forward(Coord c) {
	return _state.forward(c);
}

int CheckersStateExt::seq() const {
	return _seq;
}

// 
const CheckersState& CheckersStateExt::state() const {
	return _state;
}

const CheckersGameOptions& CheckersStateExt::options() const {
	return _options;
}


void CheckersStateExt::showFinishInfo(CheckersFinishReason reason) const {
	_logger->info("\n{}", _state.showBoard());

	std::string used_model;
	for (const auto& i : _using_models) {
		used_model += std::to_string(i) + ", ";
	}

	_logger->info(
			"[game_id:{};seq:{}] Used_model: {}",
			_game_idx,
			_seq,
			used_model);

	switch (reason) {
		case CHECKERS_MAX_STEP:
			_logger->info(
				"Ply: {} exceeds thread_state. Restarting the game(Draw++)", 
				_state.getPly());
			break;
		case CHEKCERS_BLACK_WIN:
			_logger->info("{}Black{} win at {} move", 
				GREEN_C, 
				COLOR_END, 
				_state.getPly());
			break;
		case CHEKCERS_WHITE_WIN:
			_logger->info("{}White{} win at {} move", 
				RED_C, 
				COLOR_END, 
				_state.getPly());
			break;
	}

	_logger->info(
		"Reward: {}",
		_state.getFinalValue());

}

void CheckersStateExt::saveCurrentTree(const std::string& tree_info) const {
	// Dump the tree as well.
	std::string filename = _options.dump_record_prefix + "_gameidx_" +
			std::to_string(_game_idx) + "_seq_" + std::to_string(_seq) + "_move_" +
			std::to_string(_state.getPly()) + ".tree";
	std::ofstream oo(filename);

	oo << _state.showBoard() << std::endl;
	oo << tree_info;
}

