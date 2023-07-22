// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <Python.h>
#include "netlist/netlist.h"

extern PyTypeObject PyNetsType;

namespace Python
{
    PyObject* toPythonAsNets(std::shared_ptr<ChipDB::Netlist> netlistPtr);
};
