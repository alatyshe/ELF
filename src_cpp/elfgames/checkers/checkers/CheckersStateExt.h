#pragma once

#include <set>

// ELF
#include "elf/ai/tree_search/tree_search_base.h"
#include "elf/logging/IndexedLoggerFactory.h"

#include "CheckersState.h"
#include "CheckersFeature.h"
#include "CheckersGameOptions.h"

// Game options
#include "../common/record.h"
#include "../sgf/sgf.h"
#include "Record.h"

enum CheckersFinishReason {
	CHECKERS_MAX_STEP = 0,
	CHEKCERS_BLACK_WIN,
	CHEKCERS_WHITE_WIN,
};




struct CheckersStateExt {
 public:
	// Ok
	CheckersStateExt(int game_idx, const CheckersGameOptions& options)
			: _game_idx(game_idx),
				_last_move_for_the_game(M_INVALID),
				_last_value(0.0),
				_options(options),
				_logger(
						elf::logging::getIndexedLogger("CheckersStateExt-", "")) 
				{
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		restart();
	}

	// Ok
	void   setRequest(const MsgRequest& request) {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		_curr_request = request;

		const auto& ctrl = request.client_ctrl;
	}

	// Ok
	void   addCurrentModel() {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		if (_curr_request.vers.black_ver >= 0)
			using_models_.insert(_curr_request.vers.black_ver);
		if (_curr_request.vers.white_ver >= 0)
			using_models_.insert(_curr_request.vers.white_ver);
	}

	// Ok
	const  MsgRequest& currRequest() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		return _curr_request;
	}

	// ??????????????????????????????????????????????
	// ??????????????????????????????????????????????
	// ??????????????????????????????????????????????
	float  setFinalValue(CheckersFinishReason reason, std::mt19937* rng) {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		float final_value = 0.0;

		final_value = _state.evaluate();
		_state.setFinalValue(final_value);
		return final_value;
	}

	// Ok
	Coord  lastMove() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		if (_state.justStarted())
			return _last_move_for_the_game;
		else
			return _state.lastMove();
	}

	// Ok
	void   restart() {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		_last_value = _state.getFinalValue();
		_state.reset();
		_mcts_policies.clear();
		_predicted_values.clear();

		using_models_.clear();

		_seq++;

		addCurrentModel();
	}


	// ??????????????????????????????????????????????
	// ??????????????????????????????????????????????
	// ??????????????????????????????????????????????
	CheckersRecord dumpRecord() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		CheckersRecord r;

		r.timestamp = elf_utils::sec_since_epoch_from_now();
		r.thread_id = _game_idx;
		r.seq = _seq;
		r.request = _curr_request;

		r.result.reward = _state.getFinalValue();
		// записываем все хода в строку и отправляет json
		r.result.content = coords2str(_state.getAllMoves());
		r.result.using_models =
				std::vector<int64_t>(using_models_.begin(), using_models_.end());
		r.result.policies = _mcts_policies;
		r.result.num_move = _state.getPly() - 1;
		r.result.values = _predicted_values;

		// std::cout << "GoStateExtOffline::dumpRecord!!!!!!!!!!!" << std::endl;
		// std::cout << "movesmovesmovesmovesmovesmovesmovesmovesmovesmovesmovesmoves" << std::endl;
		// std::cout << r.result.content << std::endl;
		// std::cout << "r.info()r.info()r.info()r.info()r.info()r.info()r.info()" << std::endl;
		// std::vector<Coord> _moves = _state.getAllMoves();
		// for (const Coord& c : _moves) {
		//   std::cout << "[" << c << "] ";
		// }
		// std::cout << std::endl;
		std::cout << r.info() << std::endl << std::endl;
		// std::cout << "=======================================" << std::endl;
		// std::cout << "=======================================" << std::endl;

		return r;
	}


	// Ok
	ThreadState getThreadState() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		ThreadState s;
		s.thread_id = _game_idx;
		s.seq = _seq;
		s.move_idx = _state.getPly() - 1;
		s.black = _curr_request.vers.black_ver;
		s.white = _curr_request.vers.white_ver;
		return s;
	}



	// void   saveCurrentTree(const std::string& tree_info) const {
	//   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

	//   // Dump the tree as well.
	//   std::string filename = _options.dump_record_prefix + "_" +
	//       std::to_string(_game_idx) + "_" + std::to_string(_seq) + "_" +
	//       std::to_string(_state.getPly()) + ".tree";
	//   std::ofstream oo(filename);
	//   oo << _state.showBoard() << std::endl;
	//   oo << tree_info;
	// }


	// Ok
	float  getLastGameFinalValue() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		return _last_value;
	}

	// ??????????????????????????????????????????????
	// ??????????????????????????????????????????????
	// ??????????????????????????????????????????????  
	// ПЕРЕПРОВЕРЬ ЭТУ ШТУКУ 300 раз!!!!!!!!!!!
	void   addMCTSPolicy(
			const elf::ai::tree_search::MCTSPolicy<Coord>& mcts_policy) {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		const auto& policy = mcts_policy.policy;

		// First find the max value
		float max_val = 0.0;
		for (size_t k = 0; k < policy.size(); k++) {
			const auto& entry = policy[k];
			max_val = std::max(max_val, entry.second);
		}

		_mcts_policies.emplace_back();
		std::fill(
				_mcts_policies.back().prob,
				_mcts_policies.back().prob + TOTAL_NUM_ACTIONS,
				0);
		for (size_t k = 0; k < policy.size(); k++) {
			const auto& entry = policy[k];
			unsigned char c =
					static_cast<unsigned char>(entry.second / max_val * 255);
			_mcts_policies.back().prob[entry.first] = c;
		}
	}

	// Ok
	void   addPredictedValue(float predicted_value) {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		_predicted_values.push_back(predicted_value);
	}


	// float  getLastPredictedValue() const {
	//   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

	//   if (_predicted_values.empty())
	//     return 0.0;
	//   else
	//     return _predicted_values.back();
	// }

	// Ok
	void   showFinishInfo(CheckersFinishReason reason) const;

	// Ok
	bool   forward(Coord c) {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		return _state.forward(c);
	}

	// Ok
	const  CheckersState& state() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		return _state;
	}

	// Ok
	int    seq() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		return _seq;
	}

	// bool   finished() const {
	//   display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

	//   return _options.num_games_per_thread > 0 &&
	//       _seq >= _options.num_games_per_thread;
	// }

	// Ok
	const  CheckersGameOptions& options() const {
		display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

		return _options;
	}


 protected:
	const int 	        _game_idx;
	int			            _seq = 0;

	CheckersState	      _state;
	int 		            _last_move_for_the_game;

	MsgRequest          _curr_request;
	std::set<int64_t>   using_models_;

	float               _last_value;
	
	CheckersGameOptions         _options;

	std::vector<CheckersCoordRecord> _mcts_policies;
	std::vector<float>  _predicted_values;

	std::shared_ptr<spdlog::logger> _logger;
};


















// для тренировки нейросети, работает на стороне server
class CheckersStateExtOffline {
 public:
	friend class GoFeature;

	CheckersStateExtOffline(int game_idx, const CheckersGameOptions& options)
			: _game_idx(game_idx),
				_bf(_state),
				_options(options),
				_logger(elf::logging::getIndexedLogger(
						"CheckersStateExtOffline-",
						"")) 
				{
		display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[1;36;40m");
	}

	void   fromRecord(const CheckersRecord& r) {
		display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[1;36;40m");
		
		_offline_all_moves = str2coords(r.result.content);
		_offline_winner = r.result.reward > 0 ? 1.0 : -1.0;

		_mcts_policies = r.result.policies;
		_curr_request = r.request;
		_seq = r.seq;
		_predicted_values = r.result.values;
		_state.reset();

		// std::cout << "GoStateExtOffline::fromRecord!!!!!!!!!!!" << std::endl;
		// std::cout << r.result.content << std::endl;
		// std::cout << "movesmovesmovesmovesmovesmovesmovesmovesmovesmovesmovesmoves" << std::endl;
		// for (const Coord& c : _offline_all_moves) {
		//   std::cout << "[" << c << "] ";
		// }
		// std::cout << std::endl;
		// std::cout << "r.info()r.info()r.info()r.info()r.info()r.info()r.info()" << std::endl;
		// std::cout << r.info() << std::endl << std::endl;
		// std::cout << "=======================================" << std::endl;
		// std::cout << "=======================================" << std::endl;
	}

	bool   switchRandomMove(std::mt19937* rng) {
		display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[1;36;40m");

		// Random sample one move
		if ((int)_offline_all_moves.size() <= _options.checkers_num_future_actions - 1) {
			_logger->warn(
					"[{}] #moves {} smaller than {} - 1",
					_game_idx,
					_offline_all_moves.size(),
					_options.checkers_num_future_actions);
			exit(0);
			return false;
		}
		size_t move_to = (*rng)() %
				(_offline_all_moves.size() - _options.checkers_num_future_actions + 1);
		switchBeforeMove(move_to);
		return true;
	}

	void   switchBeforeMove(size_t move_to) {
		display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[1;36;40m");

		assert(move_to < _offline_all_moves.size());

		_state.reset();
		for (size_t i = 0; i < move_to; ++i) {
			_state.forward(_offline_all_moves[i]);
		}
	}

	int  getNumMoves() const {
		display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[1;36;40m");

		return _offline_all_moves.size();
	}

	float getPredictedValue(int move_idx) const {
		display_debug_info("CheckersStateExtOffline", __FUNCTION__, "\x1b[1;36;40m");
		
		return _predicted_values[move_idx];
	}

 private:
	const int							_game_idx;
	CheckersState					_state;
	CheckersFeature				_bf;
	CheckersGameOptions 	_options;

	int _seq;
	MsgRequest						_curr_request;

	std::vector<Coord>		_offline_all_moves;
	float									_offline_winner;

	std::vector<CheckersCoordRecord> 	_mcts_policies;
	std::vector<float>		_predicted_values;

	std::shared_ptr<spdlog::logger> _logger;
};
