// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "common/logging.h"
#include "ppmwriter.h"

bool LunaCore::PPM::write(std::ostream &os, const std::vector<RGB> &data, std::size_t width)
{
    if (!os.good()) return false;
    if (width == 0) return false;

    const int maxColorValue = 255;
    const std::size_t height = data.size() / width;

    os << "P6 " << width << " " << height << " " << maxColorValue << "\n";

    for(int y=0; y<height; ++y)
    {
        auto h_offset = (height - y - 1) * width;
        for(int x=0; x<width; ++x)
        {
            os.write(reinterpret_cast<const char*>(&data.at(h_offset + x)), 3);
        }
    }       

    return true;
}
