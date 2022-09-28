// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
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
    
    const int width  = 256;
    const int height = 256;
    std::vector<LunaCore::PPM::RGB> bitmap(256*256, {0,0,0,0});

    for(int x=0; x<width; x++)
    {
        bitmap.at(x) = {255,255,255,0};
    }

    std::ofstream ofile("test/files/results/test256x256.ppm");
    BOOST_REQUIRE(ofile.good());
    BOOST_REQUIRE(ofile.is_open());

    BOOST_CHECK(LunaCore::PPM::write(ofile, bitmap, width));
    ofile.close();

    // check if we can read our own output..
    std::ifstream ifile("test/files/results/test256x256.ppm");
    BOOST_REQUIRE(ifile.good());
    BOOST_REQUIRE(ifile.is_open());    

    auto bm = LunaCore::PPM::read(ifile);
    BOOST_CHECK(bm);
    BOOST_CHECK(bm->m_width==256);
    BOOST_CHECK(bm->m_height==256);
    
    BOOST_CHECK(bm->m_data == bitmap);
}

BOOST_AUTO_TEST_SUITE_END()