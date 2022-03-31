/*  LunaPnR Source Code
 
    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once
//#include "../converters.h"
#include "celllib/cell.h"

extern PyTypeObject PyCellType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::Cell> cellPtr);
};
