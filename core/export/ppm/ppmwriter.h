// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <iostream>
#include <sstream>
#include <string>
#include "ppm.h"

namespace LunaCore::PPM
{
    bool write(std::ostream &os, const Bitmap &bm);
    bool write(const std::string &filename, const Bitmap &bm);
};
