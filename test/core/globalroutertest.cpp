// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"
#include "testhelpers.h"

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

/** expose private methods of the LunaCore::GlobalRouter::Router to make things testable */
class TestableRouter : public LunaCore::GlobalRouter::Router
{
public:

    void updateCapacity(const LunaCore::GlobalRouter::SegmentList &segments) const
    {
        return LunaCore::GlobalRouter::Router::updateCapacity(segments);
    }

    std::optional<LunaCore::GlobalRouter::SegmentList> routeTwoPointRoute(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2)
    {
        return LunaCore::GlobalRouter::Router::routeTwoPointRoute(p1,p2);
    }

    auto& at(const ChipDB::Coord64 &pos)
    {
        return m_grid->at(pos);
    }
};

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

    Logging::logInfo("Found the following Nangate sites:\n");
    for(auto site : design.m_techLib->sites())
    {
        Logging::logInfo("  Site: %s\n", site->name().c_str());
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

    TestableRouter router;

    // check for a simple vertical route
    router.createGrid(100,100,{1,1}, 100);
    BOOST_REQUIRE(router.grid() != nullptr);
    auto result = router.routeTwoPointRoute({49,0},{49,49});

    BOOST_CHECK(result);

    auto route1bm = router.grid()->generateBitmap();
    LunaCore::PPM::write("test/files/results/route1.ppm", route1bm);

    BOOST_CHECK(Helpers::compareBitmapToPPM("test/files/ppm/route1.ppm", route1bm));

    // check that a second (horizontal) route will stop
    // early at the nearest target found.
    auto result2 = router.routeTwoPointRoute({0,49},{99,49});
    BOOST_CHECK(result2);

    auto route2bm = router.grid()->generateBitmap();
    LunaCore::PPM::write("test/files/results/route2.ppm", route2bm);

    BOOST_CHECK(Helpers::compareBitmapToPPM("test/files/ppm/route2.ppm", route2bm));

    // check that a simple vertical route will go around
    // a blocked part of the grid

    router.clearGridForNewRoute();
    router.setBlockage({50,10});
    router.setBlockage({49,10});
    router.setBlockage({48,10});

    auto result3 = router.routeTwoPointRoute({49,0},{49,49});
    BOOST_CHECK(result3);

    auto route3bm = router.grid()->generateBitmap();
    LunaCore::PPM::write("test/files/results/route3.ppm", route3bm);

    BOOST_CHECK(Helpers::compareBitmapToPPM("test/files/ppm/route3.ppm", route3bm));

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

    TestableRouter router;
    router.createGrid(1200,1200,{250,250}, 100);
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
            auto result = router.routeTwoPointRoute(p1,p2);
            BOOST_CHECK(result);
        }
        std::cout << "*" << std::flush;
    }

    std::cout << "\n";

    auto bitmap = router.grid()->generateBitmap();
    LunaCore::PPM::write("test/files/results/complexroute.ppm", bitmap);

    std::ofstream ofile("test/files/results/complexroute.svg");
    LunaCore::Prim::toSVG(ofile, tree);

    //compare resulting route with reference
    BOOST_CHECK(Helpers::compareBitmapToPPM("test/files/ppm/complexroute.ppm", bitmap));

    Logging::setLogLevel(logLevel);
}

BOOST_AUTO_TEST_CASE(global_router_test_complex2)
{
    std::cout << "--== GLOBAL ROUTER TEST (complex, high-level interface) ==--\n";

    // use PRIM to decompose the net into segments
    const std::string src{R"(65 80000 10000  106800 120000  122000 140000  130800 150000  123600 70000  151600 150000  110000 90000  126000 120000  127600 100000  127600 90000  116400 50000  126000 110000  116400 80000  113200 150000  97200 180000  95600 140000  90800 110000  90800 100000  108400 110000  108400 100000  90800 130000  95600 170000  95600 150000  108400 130000  89200 120000  71600 140000  93200 80000  71600 120000  71600 110000  174000 90000  174000 110000  174000 60000  106000 70000  71600 130000  66800 80000  71600 60000  86800 70000  71600 90000  69200 70000  70800 160000  123600 230000  123600 240000  68400 150000  71600 50000  71600 100000  70800 190000  67600 180000  66800 200000  141200 230000  141200 260000  70800 170000  93200 60000  70000 210000  123600 270000  123600 180000  122800 190000  126000 280000  123600 260000  140400 190000  126000 220000  123600 200000  126000 250000  123600 210000  143600 160000  126000 130000)"};

    auto netNodes = LunaCore::Prim::loadNetNodes(src);

    BOOST_REQUIRE(netNodes.size() == 65);

    LunaCore::GlobalRouter::Router router;
    router.createGrid(1200,1200,{250,250}, 100);
    BOOST_REQUIRE(router.grid() != nullptr);

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::VERBOSE);

    std::cout << "Routing complex net..\n";

    auto segmentList = router.routeNet(netNodes, "testnet");
    BOOST_REQUIRE(segmentList);

    // check that all the tree segments
    // have a parent except the first four
    bool regularNode = false;
    std::size_t segCounter = 0;
    ChipDB::Coord64 startPos{0,0};
    for(auto const seg : segmentList.value())
    {
        BOOST_REQUIRE(seg != nullptr);

        if (segCounter == 0)
        {
            BOOST_CHECK(seg->m_parent == nullptr);   // 1st node can never have a parent assigned!
            BOOST_CHECK(regularNode == false);
            startPos = seg->m_start;
        }
        else if (startPos != seg->m_start)
        {
            // first and remaining regular nodes should end up here.
            // they all should have a valid parent
            regularNode = false;

            //FIXME:
            //BOOST_CHECK(seg->m_parent != nullptr);
            BOOST_CHECK(segCounter > 0);    // must have at least one head node.
        }
        else
        {
            // multiple head nodes
            //FIXME:
            //BOOST_CHECK(seg->m_parent == nullptr);   // head nodes can never have a parent
            BOOST_CHECK(segCounter <= 3);           // can never have more than 4 head nodes (E, W, N, S)
        }
        segCounter++;
    }

    Logging::setLogLevel(logLevel);

    // check that all GCells have capacity 1 or less
    bool foundCellWithCapacityOne = false;
    auto const gheight = router.grid()->height();
    auto const gwidth  = router.grid()->width();
    for(auto y=0; y<gheight; y++)
    {
        for(auto x=0; x<gwidth; x++)
        {
            auto const& cell = router.grid()->at(x,y);
            BOOST_CHECK(cell.m_capacity <= 1);
            if (cell.m_capacity == 1)
            {
                foundCellWithCapacityOne = true;
            }
        }
    }

    // check that we found at least one cell of capacity == 1
    BOOST_CHECK(foundCellWithCapacityOne);

    // generate a bitmap using the returned segments and compare the output
    LunaCore::GlobalRouter::Grid replicaGrid(1200,1200, {250,250});

    for(const auto seg : segmentList.value())
    {
        BOOST_REQUIRE(seg != nullptr);

        auto pos = seg->m_start;
        auto count = seg->m_length;
        while(count > 0)
        {
            replicaGrid.at(pos).setMark();
            count--;
            switch(seg->m_dir)
            {
            case LunaCore::GlobalRouter::Direction::East:
                pos.m_x--;
                break;
            case LunaCore::GlobalRouter::Direction::West:
                pos.m_x++;
                break;
            case LunaCore::GlobalRouter::Direction::North:
                pos.m_y++;
                break;
            case LunaCore::GlobalRouter::Direction::South:
                pos.m_y--;
                break;
            }
        }
    }

    // check that all net node positions have been marked
    for(auto const& node : netNodes)
    {
        auto gcoord = replicaGrid.toGridCoord(node);
        BOOST_CHECK_MESSAGE(replicaGrid.at(gcoord).isMarked(), "gcoord = " << gcoord.m_x << " " << gcoord.m_y << "  coord = " << node.m_x << " " << node.m_y);

        auto gcoord2 = router.grid()->toGridCoord(node);
        BOOST_CHECK_MESSAGE(router.grid()->at(gcoord2).isMarked(), "gcoord2 = " << gcoord2.m_x << " " << gcoord2.m_y << "  coord2 = " << node.m_x << " " << node.m_y);
    }

    auto bitmap = replicaGrid.generateBitmap();
    LunaCore::PPM::write("test/files/results/complexroute2.ppm", bitmap);

    //compare resulting route with reference
    auto check = Helpers::compareBitmapToPPM("test/files/ppm/complexroute2.ppm", bitmap);
    BOOST_CHECK(check);

    // if the check fails, write a diff bitmap for human checking
    if (!check)
    {
        auto checkBm = LunaCore::PPM::read("test/files/ppm/complexroute2.ppm");
        BOOST_REQUIRE(checkBm);

        auto diffBm = Helpers::createDiffBitmap(bitmap, checkBm.value());
        BOOST_REQUIRE(diffBm);

        if (!LunaCore::PPM::write("test/files/results/complexroute2_diff.ppm", diffBm.value()))
        {
            std::cerr << "Error writing bitmap file test/files/results/complexroute2_diff.ppm\n";
        }
    }
}


BOOST_AUTO_TEST_CASE(global_router_parallel_routes)
{
    std::cout << "--== CHECK GLOBAL ROUTER (parallel routes) ==--\n";

    TestableRouter router;
    router.createGrid(100,100,{1,1}, 100);
    BOOST_REQUIRE(router.grid() != nullptr);

    // create two parallel routes
    auto tree1 = router.routeTwoPointRoute({25,50},{75,50});
    auto tree2 = router.routeTwoPointRoute({25,50},{75,51});

    BOOST_REQUIRE(tree1);
    BOOST_REQUIRE(tree2);

    std::cout << "  Tree1 has " << tree1->size() << " segments\n";
    std::cout << "  Tree2 has " << tree2->size() << " segments\n";
    BOOST_CHECK(tree1->size() == 1);
    BOOST_CHECK(tree2->size() == 2);
}


BOOST_AUTO_TEST_SUITE_END()
