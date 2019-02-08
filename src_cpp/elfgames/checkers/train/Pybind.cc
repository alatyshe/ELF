/**
 * Copyright (c) 2018-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "GameContext.h"

namespace elfgames {
namespace checkers {

void registerPy(pybind11::module& m) {
  namespace py = pybind11;
  auto ref = py::return_value_policy::reference_internal;

  py::class_<GameContext>(m, "GameContext")
      .def(py::init<const ContextOptions&, const CheckersGameOptions&>())
      .def("ctx", &GameContext::ctx, ref)
      .def("getParams", &GameContext::getParams)
      .def("getGame", &GameContext::getGame, ref)
      .def("getClient", &GameContext::getClient, ref)
      .def("getServer", &GameContext::getServer, ref);


  py::class_<DistriServer>(m, "DistriServer")
      .def("ServerWaitForSufficientSelfplay", &DistriServer::ServerWaitForSufficientSelfplay)
      .def("notifyNewVersion", &DistriServer::notifyNewVersion)
      .def("setInitialVersion", &DistriServer::setInitialVersion)
      .def("setEvalMode", &DistriServer::setEvalMode);


  py::class_<DistriClient>(m, "DistriClient")
      .def("setRequest", &DistriClient::setRequest)
      .def("getCheckersGameStats", &DistriClient::getCheckersGameStats, ref);

  // Also register other objects.
  PYCLASS_WITH_FIELDS(m, ContextOptions)
      .def(py::init<>())
      .def("print", &ContextOptions::print);

  PYCLASS_WITH_FIELDS(m, CheckersGameOptions)
      .def(py::init<>())
      .def("info", &CheckersGameOptions::info);

  PYCLASS_WITH_FIELDS(m, WinRateStats).def(py::init<>());

  py::class_<GameStats>(m, "GameStats")
      .def("getWinRateStats", &GameStats::getWinRateStats);
      //.def("AllGamesFinished", &GameStats::AllGamesFinished)
      //.def("restartAllGames", &GameStats::restartAllGames)
      // .def("getPlayedGames", &GameStats::getPlayedGames);


  py::class_<ClientGameSelfPlay>(m, "ClientGameSelfPlay")
      .def("showBoard", &ClientGameSelfPlay::showBoard)
      // для игры в консоле
      .def("getValidMoves", &ClientGameSelfPlay::getValidMoves);
}

} // namespace go
} // namespace elfgames
