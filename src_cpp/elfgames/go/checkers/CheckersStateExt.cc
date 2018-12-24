#include "CheckersStateExt.h"

void CheckersStateExt::showFinishInfo(CheckersFinishReason reason) const {
  display_debug_info("CheckersStateExt", __FUNCTION__, "\x1b[1;36;40m");

  _logger->info("\n{}", _state.showBoard());

  std::string used_model;
  for (const auto& i : using_models_) {
    used_model += std::to_string(i) + ", ";
  }

  _logger->info(
      "[{}:{}] Current request: {}, used_model: {}, Player_swap : {}",
      _game_idx,
      _seq,
      curr_request_.info(),
      used_model,
      curr_request_.client_ctrl.player_swap);
      

  switch (reason) {
    case CHECKERS_MAX_STEP:
      _logger->info(
          "Ply: {} exceeds thread_state. Restarting the game", _state.getPly());
      break;
    case CHEKCERS_BLACK_WIN:
      _logger->info("\x1b[6;32;40mBlack\x1b[0m win at {} move", _state.getPly());
      break;
    case CHEKCERS_WHITE_WIN:
      _logger->info("\x1b[6;31;40mWhite\x1b[0m win at {} move", _state.getPly());
      break;
  }

	_logger->info(
	  "Value: {}\n",
	  _state.getFinalValue());

}
