// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <iostream>
#include <vector>
#include <optional>

#include "export/ppm/ppmwriter.h"
#include "ppmreader.h"

namespace LunaCore::PPM
{

    struct PPMBitmap
    {
        std::size_t m_width{0};
        std::size_t m_height{0};
        std::vector<RGB> m_data;
    };

    std::optional<PPMBitmap> read(std::istream &is);

}; //namespace
