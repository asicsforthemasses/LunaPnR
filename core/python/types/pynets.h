/*  LunaPnR Source Code
 
    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once
#include <Python.h>
#include "netlist/netlist.h"

extern PyTypeObject PyNetsType;

namespace Python
{
    PyObject* toPythonAsNets(std::shared_ptr<ChipDB::Netlist> netlistPtr);
};
