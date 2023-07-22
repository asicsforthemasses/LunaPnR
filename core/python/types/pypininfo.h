// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <Python.h>
#include "celllib/pin.h"

extern PyTypeObject PyPinInfoType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::PinInfo> ptr);
};
