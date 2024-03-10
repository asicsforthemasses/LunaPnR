// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PPMTests)

BOOST_AUTO_TEST_CASE(check_PPM_IO)
{
    std::cout << "--== CHECK PPM I/O ==--\n";

    LunaCore::PPM::Bitmap bm;
    bm.m_height = 256;
    bm.m_width = 256;
    bm.m_data.resize(256*256, {0,0,0,0});

    for(int x=0; x<bm.m_width; x++)
    {
        bm.m_data.at(x) = {255,255,255,0};
    }

    BOOST_CHECK(LunaCore::PPM::write("test/files/results/test256x256.ppm", bm));

    // check if we can read our own output..
    auto checkbm = LunaCore::PPM::read("test/files/results/test256x256.ppm");
    BOOST_CHECK(checkbm);
    BOOST_CHECK(checkbm->m_width==256);
    BOOST_CHECK(checkbm->m_height==256);

    BOOST_CHECK(checkbm->m_data == bm.m_data);
}

BOOST_AUTO_TEST_SUITE_END()