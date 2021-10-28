#include "lunacore.h"

#include <sstream>
#include <fstream>
#include <random>

#include <boost/test/unit_test.hpp>

namespace tt = boost::test_tools;

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

BOOST_AUTO_TEST_CASE(check_diffusion)
{
    std::cout << "--== CHECK DIFFUSION ==--\n";

    ChipDB::Netlist netlist;
    ChipDB::Region  region;
    region.m_rect.setUR({5000,5000});

    // place multiple cells in the center cell
    // but slightly offset so we can check
    // the velocity

    auto cell = new ChipDB::Cell();
    cell->m_name = "cell";
    cell->m_size = ChipDB::Coord64{500,500};

    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

    for(int32_t ox=-1; ox<=1; ox++)
    {
        for(int32_t oy=-1; oy<=1; oy++)
        {
            auto instance = new ChipDB::Instance(cell);
            instance->m_name = "instance";
            instance->m_pos  = ChipDB::Coord64{2500 + ox*200 - cell->m_size.m_x/2, 2500 + oy*200 - cell->m_size.m_y/2};

            std::stringstream ss;
            ss << "ins_x" << ox << "_y" << oy;

            doLog(LOG_VERBOSE,"  instance %s at %d,%d\n", ss.str().c_str(), instance->m_pos.m_x, instance->m_pos.m_y);

            netlist.m_instances.add(ss.str(), instance);
        }
    }

    auto bm = LunaCore::QPlacer::createDensityBitmap(&netlist, &region, 1000, 1000);
    BOOST_CHECK(bm != nullptr);
    BOOST_CHECK(bm->width() == 5);
    BOOST_CHECK(bm->height() == 5);
    
    LunaCore::QPlacer::setMinimalDensities(bm);

    // check that the average density is 1
    double totalDensity = 0;
    for(size_t x=0; x<bm->width(); x++)
    {
        for(size_t y=0; y<bm->height(); y++)
        {
            totalDensity += bm->at(x,y);
        }
    }

    BOOST_TEST(bm->at(2,2) == 9*0.25f, tt::tolerance(0.01));
    BOOST_TEST(bm->at(2,1) == 0.9583f, tt::tolerance(0.01));
    BOOST_TEST(bm->at(2,3) == 0.9583f, tt::tolerance(0.01));

    doLog(LOG_VERBOSE, "  bm(2,2) = %f\n", bm->at(2,2));
    doLog(LOG_VERBOSE, "  bm(2,1) = %f\n", bm->at(2,1));
    doLog(LOG_VERBOSE, "  bm(2,3) = %f\n", bm->at(2,3));

    BOOST_CHECK(totalDensity > 8.0f);
    BOOST_CHECK(totalDensity < 9.0f);

    LunaCore::QPlacer::VelocityBitmap vm(bm->width(), bm->height());
    LunaCore::QPlacer::calcVelocityBitmap(bm, &vm);

    BOOST_TEST((vm.at(2,1).m_dx == 0), tt::tolerance(0.01));
    BOOST_TEST((vm.at(2,1).m_dy == 0), tt::tolerance(0.01));

    doLog(LOG_VERBOSE, "  vm(2,1) = %f,%f\n", vm.at(2,1).m_dx, vm.at(2,1).m_dy);
    doLog(LOG_VERBOSE, "  vm(2,2) = %f,%f\n", vm.at(2,2).m_dx, vm.at(2,2).m_dy);
    doLog(LOG_VERBOSE, "  vm(2,3) = %f,%f\n", vm.at(2,3).m_dx, vm.at(2,3).m_dy);

    doLog(LOG_VERBOSE, "  vm(1,2) = %f,%f\n", vm.at(1,2).m_dx, vm.at(1,2).m_dy);
    doLog(LOG_VERBOSE, "  vm(3,2) = %f,%f\n", vm.at(3,2).m_dx, vm.at(3,2).m_dy);

    setLogLevel(ll);
}


BOOST_AUTO_TEST_SUITE_END()