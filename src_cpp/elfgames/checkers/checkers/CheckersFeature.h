#pragma once

#include "CheckersBoard.h"
#include "elf/logging/IndexedLoggerFactory.h"

// история досок пока не используется
struct CheckersBoardHistory {

	std::array<int64_t, 2>	forward;
	std::array<int64_t, 2>	backward;
	std::array<int64_t, 2>	pieces;
	int64_t									empty;
	int											active;

	// просто копируем положение фишек на доске
	CheckersBoardHistory(const CheckersBoard& b) {
		display_debug_info("struct CheckersBoardHistory", __FUNCTION__, "\x1b[1;36;40m");

		forward[BLACK_PLAYER] = b.forward[BLACK_PLAYER];
		backward[BLACK_PLAYER] = b.backward[BLACK_PLAYER];
		pieces[BLACK_PLAYER] = (forward[BLACK_PLAYER]) | (backward[BLACK_PLAYER]);
		forward[WHITE_PLAYER] = b.forward[WHITE_PLAYER];
		backward[WHITE_PLAYER] = b.backward[WHITE_PLAYER];
		pieces[WHITE_PLAYER] = (forward[WHITE_PLAYER]) | (backward[WHITE_PLAYER]);
		
		empty = empty;
		active = active;
	}
};

class CheckersState;





class CheckersFeature {
 public:

	CheckersFeature(const CheckersState& s) 
			: s_(s) {
		display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[1;36;40m");
	}

	const CheckersState& state() const {
		display_debug_info("CheckersFeature", __FUNCTION__, "\x1b[1;36;40m");
		return s_;
	}

	void extract(std::vector<float>* features) const;
	void extract(float* features) const;

 private:
	const CheckersState&		s_;
	static constexpr int64_t kBoardRegion = CHECKERS_BOARD_SIZE * CHECKERS_BOARD_SIZE;

	// Compute features.
	void getPawns(int player, float* data) const;
	void getKings(int player, float* data) const;
	// bool getHistory(int player, float* data) const;
	// bool getHistoryExp(int player, float* data) const;
};





struct CheckersReply {
	const CheckersFeature&	bf;
	int											c;
	std::vector<float>			pi;
	float										value = 0;
	// Model version.
	int64_t									version = -1;

	CheckersReply(const CheckersFeature& bf) : bf(bf), pi(TOTAL_NUM_ACTIONS, 0.0) {
		display_debug_info("struct CheckersReply", __FUNCTION__, "\x1b[1;36;40m");
	}
	
	std::string   info() {
		std::stringstream ss;

		ss  << "c : " << c << std::endl
				<< "pi : " << std::endl;
		for (auto i = pi.begin(); i != pi.end(); ++i)
			ss << *i << ", ";
		ss  << std::endl
				<< "value : " << value << std::endl
				<< "version : " << version << std::endl;
		// ss  << bf.state().showBoard() << std::endl;
		return ss.str();
	}
};





