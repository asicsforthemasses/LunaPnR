// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdio>
#include <Python.h>
#include "database/database.h"

extern PyTypeObject PyPinType;

namespace Python
{
    PyObject* toPython(const ChipDB::Instance::Pin &pin);
};
