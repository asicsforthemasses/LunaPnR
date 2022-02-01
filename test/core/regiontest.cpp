/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(RegionTests)

BOOST_AUTO_TEST_CASE(check_region_creation)
{
    std::cout << "--== CHECK CREATE REGION ==--\n";
    
    int64_t totalCellWidth = 500 * 3000;
    int64_t cellHeight     = 5000;

    auto cellArea = totalCellWidth * cellHeight;

    auto region = ChipDB::createRegion(
        1.0 /* aspect ratio */,
        1000 /* minCellWidth */,
        cellHeight,
        0 /* row Distance */,
        totalCellWidth /* total cell width */
    );

    BOOST_CHECK(region != nullptr);

    auto regionArea = region->m_rect.width() * region->m_rect.height();

    BOOST_CHECK(regionArea >= cellArea);
};


BOOST_AUTO_TEST_SUITE_END()