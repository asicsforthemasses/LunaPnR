/*  LunaPnR Source Code
 
    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once
#include <Python.h>
#include "celllib/pin.h"

extern PyTypeObject PyPinInfoType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::PinInfo> ptr);
};
