/*  LunaPnR Source Code
 
    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once
#include <Python.h>
#include <memory>
#include "netlist/net.h"

extern PyTypeObject PyNetType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::Net> netPtr);
};
