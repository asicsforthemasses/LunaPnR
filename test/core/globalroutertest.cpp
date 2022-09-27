// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(GlobalRouterTest)

BOOST_AUTO_TEST_CASE(global_router_cell_size)
{
    std::cout << "--== GLOBAL ROUTER CELL SIZE TEST ==--\n";

    auto logLevel = Logging::getLogLevel();

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));

    Logging::setLogLevel(Logging::LogType::VERBOSE);

    LunaCore::GlobalRouter::Router grouter;

    auto site = design.m_techLib->lookupSiteInfo("core");
    BOOST_REQUIRE(site.isValid());

    auto minCellSize = site->m_size;
    BOOST_REQUIRE((minCellSize.m_x > 0));
    BOOST_REQUIRE((minCellSize.m_y > 0));

    auto gcellSize = grouter.determineGridCellSize(design, "core", 100, 100);
    BOOST_CHECK(gcellSize.has_value());

    if (!gcellSize.has_value())
    {
        Logging::setLogLevel(logLevel);
        return;
    }

    // check expected values
    BOOST_CHECK(gcellSize.value().m_x == 32000);
    BOOST_CHECK(gcellSize.value().m_y == 40000);

    // check both extents are an exact multiple of the min Cell size
    auto remain_x = gcellSize.value().m_x % minCellSize.m_x;
    auto remain_y = gcellSize.value().m_y % minCellSize.m_y;

    BOOST_CHECK(remain_x == 0);
    BOOST_CHECK(remain_y == 0);

    auto tracks = grouter.calcNumberOfTracks(design, "core", gcellSize.value());

    BOOST_CHECK(tracks.has_value());
    if (!tracks.has_value())
    {
        Logging::setLogLevel(logLevel);
        return;
    }

    BOOST_CHECK(tracks.value().horizontal == 120);
    BOOST_CHECK(tracks.value().vertical == 100);

    Logging::setLogLevel(logLevel);
};

BOOST_AUTO_TEST_CASE(global_router_cell_size2)
{
    std::cout << "--== GLOBAL ROUTER CELL SIZE 2 TEST ==--\n";

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::VERBOSE);

    std::ifstream leffile("test/files/nangate/ocl.tech.lef");
    BOOST_REQUIRE(leffile.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));

    BOOST_REQUIRE(design.m_techLib->getNumberOfSites() > 0);    

    Logging::doLog(Logging::LogType::INFO,"Found the following Nangate sites:\n");
    for(auto site : design.m_techLib->sites())
    {
        Logging::doLog(Logging::LogType::INFO, "  Site: %s\n", site->name().c_str());
    }

    auto site = design.m_techLib->lookupSiteInfo("FreePDK45_38x28_10R_NP_162NW_34O");
    BOOST_REQUIRE(site.isValid());

    auto minCellSize = site->m_size;
    BOOST_REQUIRE((minCellSize.m_x > 0));
    BOOST_REQUIRE((minCellSize.m_y > 0));

    LunaCore::GlobalRouter::Router grouter;
    auto gcellSize = grouter.determineGridCellSize(design, "FreePDK45_38x28_10R_NP_162NW_34O", 100, 100);
    BOOST_CHECK(gcellSize.has_value());

    if (!gcellSize.has_value())
    {
        Logging::setLogLevel(logLevel);
        return;
    }

    // check expected values
    BOOST_CHECK(gcellSize.value().m_x == 7030);
    BOOST_CHECK(gcellSize.value().m_y == 5600);

    // check both extents are an exact multiple of the min Cell size
    auto remain_x = gcellSize.value().m_x % minCellSize.m_x;
    auto remain_y = gcellSize.value().m_y % minCellSize.m_y;

    BOOST_CHECK(remain_x == 0);
    BOOST_CHECK(remain_y == 0);

    auto tracks = grouter.calcNumberOfTracks(design, "FreePDK45_38x28_10R_NP_162NW_34O", gcellSize.value());

    BOOST_CHECK(tracks.has_value());
    if (!tracks.has_value())
    {
        Logging::setLogLevel(logLevel);
        return;
    }

    BOOST_CHECK(tracks.value().horizontal == 110);
    BOOST_CHECK(tracks.value().vertical == 100);

    Logging::setLogLevel(logLevel);
};


BOOST_AUTO_TEST_SUITE_END()
