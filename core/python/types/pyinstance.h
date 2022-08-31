// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <Python.h>
#include "netlist/instance.h"

extern PyTypeObject PyInstanceType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::Instance> instancePtr);
};
