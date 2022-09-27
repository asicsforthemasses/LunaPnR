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

BOOST_AUTO_TEST_CASE(global_router_test_simple)
{
    std::cout << "--== GLOBAL ROUTER TEST (simple) ==--\n";

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::VERBOSE);

    LunaCore::GlobalRouter::Router router;

    // check for a simple vertical route

    router.createGrid(100,100,{1,1});
    BOOST_REQUIRE(router.grid() != nullptr);
    auto result = router.route({49,0},{49,49});
    
    BOOST_CHECK(result);

    // check that a second (horizontal) route will stop
    // early at the nearest target found.
    router.grid()->exportToPGM("route1.pgm");

    result = router.route({0,49},{99,49});
    BOOST_CHECK(result);

    router.grid()->exportToPGM("route2.pgm");

    // check that a simple vertical route will go around
    // a blocked part of the grid

    router.clearGrid();
    router.setBlockage({50,10});
    router.setBlockage({49,10});
    router.setBlockage({48,10});

    result = router.route({49,0},{49,49});
    BOOST_CHECK(result);

    router.grid()->exportToPGM("route3.pgm");

    Logging::setLogLevel(logLevel);
}

BOOST_AUTO_TEST_CASE(global_router_test_complex)
{
    std::cout << "--== GLOBAL ROUTER TEST (complex) ==--\n";

    // use PRIM to decompose the net into segments
    const std::string src{R"(65 80000 10000  106800 120000  122000 140000  130800 150000  123600 70000  151600 150000  110000 90000  126000 120000  127600 100000  127600 90000  116400 50000  126000 110000  116400 80000  113200 150000  97200 180000  95600 140000  90800 110000  90800 100000  108400 110000  108400 100000  90800 130000  95600 170000  95600 150000  108400 130000  89200 120000  71600 140000  93200 80000  71600 120000  71600 110000  174000 90000  174000 110000  174000 60000  106000 70000  71600 130000  66800 80000  71600 60000  86800 70000  71600 90000  69200 70000  70800 160000  123600 230000  123600 240000  68400 150000  71600 50000  71600 100000  70800 190000  67600 180000  66800 200000  141200 230000  141200 260000  70800 170000  93200 60000  70000 210000  123600 270000  123600 180000  122800 190000  126000 280000  123600 260000  140400 190000  126000 220000  123600 200000  126000 250000  123600 210000  143600 160000  126000 130000)"};

    auto netNodes = LunaCore::Prim::loadNetNodes(src);

    BOOST_REQUIRE(netNodes.size() == 65);

    auto tree = LunaCore::Prim::prim(netNodes);
    BOOST_CHECK(tree.size() == 65);

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::VERBOSE);

    LunaCore::GlobalRouter::Router router;
    router.createGrid(1200,1200,{250,250});
    BOOST_REQUIRE(router.grid() != nullptr);

    // iterate over the tree edges and route them
    std::cout << "Routing complex net..\n";
    std::size_t nodes = tree.size();

    for(auto const& treeNode : tree)
    {
        auto p1 = treeNode.m_pos;
        for(auto const& edge : treeNode.m_edges)
        {
            auto p2 = edge.m_pos;
            auto result = router.route(p1,p2);
            BOOST_CHECK(result);
        }
        std::cout << "*" << std::flush;
    }

    std::cout << "\n";

    router.grid()->exportToPGM("complexroute.pgm");

    std::ofstream ofile("complexroute.svg");
    LunaCore::Prim::toSVG(ofile, tree);

    Logging::setLogLevel(logLevel);
}


BOOST_AUTO_TEST_SUITE_END()
