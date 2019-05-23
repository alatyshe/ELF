#pragma once

#include <time.h>

#include <iostream>
#include <memory>
#include <vector>

// elf
#include "elf/base/context.h"
#include "elf/legacy/python_options_utils_cpp.h"
// checkers
#include "../common/record.h"
#include "../game/Record.h"
#include "data_loader.h"

inline elf::shared::Options getNetOptions(
    const ContextOptions& context_options,
    const CheckersGameOptions& game_options) {
  elf::shared::Options netOptions;
  
  netOptions.addr =
      game_options.server_addr == "" ? "localhost" : game_options.server_addr;
  netOptions.port = game_options.port;
  netOptions.use_ipv6 = true;
  netOptions.verbose = game_options.verbose;
  netOptions.identity = context_options.job_id;

  return netOptions;
}
