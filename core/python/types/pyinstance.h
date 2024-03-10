// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cstdio>
#include <Python.h>
#include "database/database.h"

extern PyTypeObject PyInstanceType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::Instance> instancePtr);
};
