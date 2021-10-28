#include "lunacore.h"

#include <sstream>
#include <fstream>
#include <random>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PlacementBitmapTest)

BOOST_AUTO_TEST_CASE(check_densitybitmap)
{
    std::cout << "--== CHECK DENSITY BITMAP ==--\n";

    LunaCore::QPlacer::DensityBitmap bm(255,255);

    // test that all pixels are zero
    for(ssize_t y=0; y<bm.height(); y++)
    {
        for(ssize_t x=0; x<bm.width(); x++)
        {
            BOOST_CHECK(bm.at(x,y) == 0.0);
        }
    }

    // check that out of bounds coordinates return zero
    BOOST_CHECK(bm.at(-1,0) == 0.0);
    BOOST_CHECK(bm.at(0,-1) == 0.0);
    BOOST_CHECK(bm.at(bm.width(),0) == 0.0);
    BOOST_CHECK(bm.at(0,bm.height()) == 0.0);    
}

BOOST_AUTO_TEST_CASE(check_velocitybitmap)
{
    std::cout << "--== CHECK VELOCITY BITMAP ==--\n";

    LunaCore::QPlacer::VelocityBitmap bm(255,255);

    auto zeroVelocity = LunaCore::QPlacer::Velocity(0);

    // test that all pixels are zero
    for(ssize_t y=0; y<bm.height(); y++)
    {
        for(ssize_t x=0; x<bm.width(); x++)
        {
            BOOST_CHECK(bm.at(x,y) == zeroVelocity);
        }
    }

    // check that out of bounds coordinates return zero
    BOOST_CHECK(bm.at(-1,0) == zeroVelocity);
    BOOST_CHECK(bm.at(0,-1) == zeroVelocity);
    BOOST_CHECK(bm.at(bm.width(),0) == zeroVelocity);
    BOOST_CHECK(bm.at(0,bm.height()) == zeroVelocity);
}

BOOST_AUTO_TEST_CASE(check_createdensitybitmap)
{
    std::cout << "--== CHECK DENSITY BITMAP ==--\n";


    ChipDB::Netlist netlist;
    ChipDB::Region  region;
    region.m_rect.setUR({2000,2000});

    // place one cell of size 1000,1000 with it's center at
    // 1000,1000
    //

    auto cell = new ChipDB::Cell();
    cell->m_name = "cell";
    cell->m_size = ChipDB::Coord64{1000,1000};

    auto instance = new ChipDB::Instance(cell);
    instance->m_name = "instance";
    instance->m_pos  = ChipDB::Coord64{500,500};
    netlist.m_instances.add("ins1", instance);

    auto bm = LunaCore::QPlacer::createDensityBitmap(&netlist, &region, 1000, 1000);

    BOOST_CHECK(bm != nullptr);

    BOOST_CHECK(bm->width() == 2);
    BOOST_CHECK(bm->height() == 2);

    auto ll = getLogLevel();
    //setLogLevel(LOG_VERBOSE);

    doLog(LOG_VERBOSE, "  bm(0,0) = %f\n", bm->at(0,0));
    doLog(LOG_VERBOSE, "  bm(0,1) = %f\n", bm->at(0,1));
    doLog(LOG_VERBOSE, "  bm(1,0) = %f\n", bm->at(1,0));
    doLog(LOG_VERBOSE, "  bm(1,1) = %f\n", bm->at(1,1));

    setLogLevel(ll);

    BOOST_CHECK(bm->at(0,0) == 0.25);
    BOOST_CHECK(bm->at(0,1) == 0.25);
    BOOST_CHECK(bm->at(1,0) == 0.25);
    BOOST_CHECK(bm->at(1,1) == 0.25);

}

BOOST_AUTO_TEST_SUITE_END()