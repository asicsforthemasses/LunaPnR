// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <fstream>
#include "common/logging.h"
#include "ppmwriter.h"

bool LunaCore::PPM::write(const std::string &filename, const Bitmap &bm)
{
    std::ofstream ofile(filename);
    if (!ofile.good())
    {
        return false;
    }
    if (!ofile.is_open())
    {
        return false;
    }

    return write(ofile, bm);
}

bool LunaCore::PPM::write(std::ostream &os, const Bitmap &bm)
{
    if (!os.good()) return false;
    if (bm.m_width == 0) return false;

    const int maxColorValue = 255;

    os << "P6 " << bm.m_width << " " << bm.m_height << " " << maxColorValue << "\n";

    for(int y=0; y<bm.m_height; ++y)
    {
        auto h_offset = (bm.m_height - y - 1) * bm.m_width;
        for(int x=0; x<bm.m_width; ++x)
        {
            os.write(reinterpret_cast<const char*>(&bm.m_data.at(h_offset + x)), 3);
        }
    }

    return true;
}
