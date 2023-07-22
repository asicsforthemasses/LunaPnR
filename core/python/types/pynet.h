// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <Python.h>
#include <memory>
#include "netlist/net.h"

extern PyTypeObject PyNetType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::Net> netPtr);
};
