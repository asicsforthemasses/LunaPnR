// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <Python.h>
#include "techlib/techlib.h"

extern PyTypeObject PySiteInfoType;

namespace Python
{
    PyObject* toPython(std::shared_ptr<ChipDB::SiteInfo> sitePtr);
};
