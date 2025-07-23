// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TechlibTest)

BOOST_AUTO_TEST_CASE(various_tech_tests)
{
    std::cout << "--== TECHLIB TEST ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    auto techlib = design.m_techLib;

    BOOST_CHECK(techlib->getNumberOfLayers() > 0);

    // iterate over layers
    std::cout << "Iterating over layers:\n";

    auto iter = techlib->layers().begin();
    while(iter != techlib->layers().end())
    {
        std::cout << "  Layer: " << iter->name() << "\n";
        iter++;
    }

    BOOST_CHECK(techlib->getNumberOfSites() > 0);

    // iterate over sites
    std::cout << "Iterating over sites:\n";
    auto iter2 = techlib->sites().begin();
    while(iter2 != techlib->sites().end())
    {
        std::cout << "  Site: " << iter2->name() << "\n";
        iter2++;
    }
}

BOOST_AUTO_TEST_SUITE_END()
