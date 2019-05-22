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

#include "../train/GameContext.h"

namespace elfgames {
namespace checkers {

void registerPy(pybind11::module& m) {
  namespace py = pybind11;
  auto ref = py::return_value_policy::reference_internal;

  py::class_<GameContext>(m, "GameContext")
      .def(py::init<const ContextOptions&, const GameOptions&>())
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
      .def("getGameStats", &DistriClient::getGameStats, ref);

  // Also register other objects.
  PYCLASS_WITH_FIELDS(m, ContextOptions)
      .def(py::init<>())
      .def("print", &ContextOptions::print);

  PYCLASS_WITH_FIELDS(m, GameOptions)
      .def(py::init<>())
      .def("info", &GameOptions::info);

  PYCLASS_WITH_FIELDS(m, WinRateStats).def(py::init<>());

  py::class_<GameStats>(m, "GameStats")
      .def("getWinRateStats", &GameStats::getWinRateStats);


  py::class_<ClientGameSelfPlay>(m, "ClientGameSelfPlay")
       // rl_checkers server methods
      .def("getBoard", &ClientGameSelfPlay::getBoard)

      .def("showBoard", &ClientGameSelfPlay::showBoard)
      .def("getCurrentPlayer", &ClientGameSelfPlay::getCurrentPlayer)
      // для игры в консоле
      .def("getValidMoves", &ClientGameSelfPlay::getValidMoves);
}

} // namespace go
} // namespace elfgames
