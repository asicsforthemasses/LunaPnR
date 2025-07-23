// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <limits>
#include <stdexcept>
#include <cmath>
#include "lunacore.h"

namespace Helpers
{

inline bool compare(const float &v1, const float &v2)
{
    return std::fabs(v1-v2) <= std::numeric_limits<float>::epsilon();
}

constexpr bool compare(const double &v1, const double &v2)
{
    //note: make sure it can be a constexpr by not using std::fabs
    if (v1 > v2)
    {
        return (v2-v1) <= std::numeric_limits<double>::epsilon();
    }
    else
    {
        return (v1-v2) <= std::numeric_limits<double>::epsilon();
    }
    //return std::fabs(v1-v2) <= std::numeric_limits<double>::epsilon();
}

inline bool compareBitmapToPPM(const std::string &filename, const LunaCore::PPM::Bitmap &bitmap)
{
    std::ifstream ifile(filename);
    if (!ifile.good())
    {
        throw std::runtime_error("compareBitmapToPPM: cannot open file for reading.");
    }

    auto bm = LunaCore::PPM::read(ifile);
    if (!bm)
    {
        throw std::runtime_error("compareBitmapToPPM: cannot read PPM bitmap.");
    }

    if (bm->m_data != bitmap.m_data)
    {
        for(std::size_t offset=0; offset<bitmap.m_data.size(); offset++)
        {
            auto pixel1 = bm->m_data.at(offset);
            auto pixel2 = bitmap.m_data.at(offset);
            if (pixel1 != pixel2)
            {
                std::cout << " offset: " << offset << " pixel1: " << pixel1 << " pixel2: " << pixel2 << "\n";
            }
        }
    }

    return (bm->m_width == bitmap.m_width) && (bm->m_height == bitmap.m_height) &&
        (bm->m_data == bitmap.m_data);
}

inline std::optional<LunaCore::PPM::Bitmap> createDiffBitmap(const LunaCore::PPM::Bitmap &bm1, const LunaCore::PPM::Bitmap &bm2)
{
    if (bm1.m_height != bm2.m_height) return std::nullopt;
    if (bm1.m_width  != bm2.m_width) return std::nullopt;

    LunaCore::PPM::Bitmap diff;
    diff.m_width = bm1.m_width;
    diff.m_height = bm1.m_height;
    diff.m_data.resize(bm1.m_data.size());

    for(std::size_t offset=0; offset<bm1.m_data.size(); offset++)
    {
        auto pixel1 = bm1.m_data.at(offset);
        auto pixel2 = bm2.m_data.at(offset);
#if 0
        uint8_t d_r = static_cast<uint8_t>(std::abs(static_cast<int>(pixel1.r) - static_cast<int>(pixel2.r)));
        uint8_t d_g = static_cast<uint8_t>(std::abs(static_cast<int>(pixel1.g) - static_cast<int>(pixel2.g)));
        uint8_t d_b = static_cast<uint8_t>(std::abs(static_cast<int>(pixel1.b) - static_cast<int>(pixel2.b)));

        diff.m_data.at(offset) = LunaCore::PPM::RGB{d_r,d_g,d_b,0};
#endif

        if (pixel1 == pixel2)
        {
            diff.m_data.at(offset) = LunaCore::PPM::RGB{0,0,0,0};
        }
        else
        {
            diff.m_data.at(offset) = LunaCore::PPM::RGB{255,255,255,0};
        }
    }

    return diff;
}

};