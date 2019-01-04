// /**
//  * Copyright (c) 2018-present, Facebook, Inc.
//  * All rights reserved.
//  *
//  * This source code is licensed under the BSD-style license found in the
//  * LICENSE file in the root directory of this source tree.
//  */

// #include <pybind11/pybind11.h>
// #include <pybind11/stl.h>
// #include <pybind11/stl_bind.h>

// #include "game_context.h"

// namespace elfgames {
// namespace go {

// void registerPy(pybind11::module& m) {
//   namespace py = pybind11;
//   auto ref = py::return_value_policy::reference_internal;

//   // py::class_<GameContext>(m, "GameContext")
//   //     .def(py::init<const ContextOptions&, const CheckersGameOptions&>())
//   //     .def("ctx", &GameContext::ctx, ref)
//   //     .def("getParams", &GameContext::getParams)
//   //     .def("getGame", &GameContext::getGame, ref)
//   //     .def("setRequest", &GameContext::setRequest);

//   // Also register other objects.
//   PYCLASS_WITH_FIELDS(m, ContextOptions)
//       .def(py::init<>())
//       .def("print", &ContextOptions::print);

//   PYCLASS_WITH_FIELDS(m, CheckersGameOptions)
//       .def(py::init<>())
//       .def("info", &CheckersGameOptions::info);



// // закоментил за ненадобностью, чтобы перед глазами не маячило
// // (откоментить, нужно для игры в консоле)
//   py::class_<GameSelfPlay>(m, "GameSelfPlay")
//       .def("showBoard", &GameSelfPlay::showBoard)
//       // для игры в консоле
//       // .def("getNextPlayer", &GameSelfPlay::getNextPlayer)
//       .def("getValidMoves", &GameSelfPlay::getValidMoves)
//       .def("getLastMove", &GameSelfPlay::getLastMove);
//       // .def("getScore", &GameSelfPlay::getScore);
//       // .def("getLastScore", &GameSelfPlay::getLastScore);
// }

// } // namespace go
// } // namespace elfgames
