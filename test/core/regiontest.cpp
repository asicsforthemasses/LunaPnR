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

BOOST_AUTO_TEST_SUITE(RegionTests)

BOOST_AUTO_TEST_CASE(check_region_creation)
{
    std::cout << "--== CHECK CREATE REGION ==--\n";
    
    ChipDB::Size64 minCellSize{800,10000};
    ChipDB::Rect64 extents{{0,0},{100000,100000}};

    const int expectedRowCount = std::floor(extents.getSize().m_y / minCellSize.m_y);

    auto regionWithoutHalo = ChipDB::createRegion("Core",
        extents,
        minCellSize
    );

    BOOST_CHECK(regionWithoutHalo != nullptr);
    BOOST_CHECK(regionWithoutHalo->m_rows.size() == expectedRowCount);
    BOOST_CHECK(regionWithoutHalo->getPlacementSize() == extents.getSize());

    ChipDB::Margins64 margins{minCellSize.m_y ,minCellSize.m_y, minCellSize.m_x, minCellSize.m_x};
    auto regionWithHalo = ChipDB::createRegion("Core",
        extents,
        minCellSize,
        margins
    );

    std::cout << "Placement size without halo: " << regionWithoutHalo->getPlacementSize() << "\n";
    std::cout << "Placement size with halo   : " << regionWithHalo->getPlacementSize() << "\n";

    BOOST_CHECK(regionWithHalo->m_rows.size() == (expectedRowCount-2));
    BOOST_CHECK(regionWithHalo->getPlacementSize().m_x == (regionWithoutHalo->getPlacementSize().m_x-2*minCellSize.m_x));
    BOOST_CHECK(regionWithHalo->getPlacementSize().m_y == (regionWithoutHalo->getPlacementSize().m_y-2*minCellSize.m_y));
};


BOOST_AUTO_TEST_SUITE_END()