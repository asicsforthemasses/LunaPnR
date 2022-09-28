// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
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

    return (bm->m_width == bitmap.m_width) && (bm->m_height == bitmap.m_height) && 
        (bm->m_data == bitmap.m_data);
}

};