
// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <fstream>
#include <regex>
#include "ppmreader.h"
#include "common/logging.h"


std::optional<LunaCore::PPM::Bitmap> LunaCore::PPM::read(const std::string &filename)
{
    std::ifstream ifile(filename);
    if (!ifile.good())
    {
        return std::nullopt;
    }

    return read(ifile);
}

std::optional<LunaCore::PPM::Bitmap>
    LunaCore::PPM::read(std::istream &is)
{
    if (!is.good()) return std::nullopt;

    // assume the complete header is on one line
    // and there are no comments

    std::regex re_Header(R"(\s*P6\s*(\d*)\s*(\d*)\s*\d*)");

    std::string headerString;
    std::getline(is, headerString);

    if (headerString.empty())
    {
        return std::nullopt;
    }

    std::smatch matches;
    if (!std::regex_search(headerString, matches, re_Header))
    {
        Logging::logError("PPM::read could not find a valid header\n");
        return std::nullopt;
    }
    else
    {
        auto width  = std::stoul(matches.str(1));
        auto height = std::stoul(matches.str(2));
        Logging::logVerbose("Loading PPM %lu x %lu\n",
            width, height);

        // some arbitrary sanity checking..
        if ((width > 10240) || (height > 10240))
        {
            Logging::logError("PPM::read PPM too big! (max width or height is 10240.\n");
            return std::nullopt;
        }

        LunaCore::PPM::Bitmap bm;
        bm.m_width  = width;
        bm.m_height = height;
        bm.m_data.resize(width*height);

        for(auto y=0; y<height; y++)
        {
            for(auto x=0; x<width; x++)
            {
                RGB pixel;
                is.read((char*)&pixel, 3);
                bm.m_data.at(x + (height-y-1)*width) = pixel;
            }
        }
        return std::move(bm);
    }

    return std::nullopt;
}