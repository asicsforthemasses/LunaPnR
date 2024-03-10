// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <optional>

#include "export/ppm/ppm.h"

namespace LunaCore::PPM
{
    std::optional<Bitmap> read(std::istream &is);
    std::optional<Bitmap> read(const std::string &filename);

}; //namespace
