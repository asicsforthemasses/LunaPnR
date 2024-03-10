// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <string>
#include <string_view>
#include "database/database.h"

namespace Python
{

// ********************************************************************************
//
//   toPython
//
//   The toPython template function converts a C++ type and exposes it to Python.
//
// ********************************************************************************

PyObject* toPython(const int &t);

PyObject* toPython(const unsigned int &t);

PyObject* toPython(const float &t);

PyObject* toPython(const double &t);

PyObject* toPython(const size_t &t);

PyObject* toPython(const ssize_t &t);

PyObject* toPython(const std::string_view &t);

PyObject* toPython(const std::string &t);

PyObject* toPython(const char *t);

PyObject* toPython(const ChipDB::Coord64 &t);

PyObject* toPython(const ChipDB::Rect64 &t);

PyObject* toPython(const ChipDB::CellClass &t);

PyObject* toPython(const ChipDB::CellSubclass &t);

PyObject* toPython(const ChipDB::SymmetryFlags &t);

// ********************************************************************************
//   fromPython
// ********************************************************************************

bool fromPython(PyObject *obj, int &result);

bool fromPython(PyObject *obj, std::string &result);

};