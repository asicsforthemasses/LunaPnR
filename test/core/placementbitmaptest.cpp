/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


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

    auto cell = std::make_shared<ChipDB::Cell>("cell");
    cell->m_size = ChipDB::Coord64{1000,1000};

    auto instance = std::make_shared<ChipDB::Instance>("instance", cell);
    instance->m_pos  = ChipDB::Coord64{500,500};
    netlist.m_instances.add(instance);

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

    const int64_t xsize = 5000;
    const int64_t ysize = 5000;
    const int64_t bitmapCellSize = 1000;

    region.m_rect.setUR({xsize,ysize});

    // place multiple cells in the center cell
    // but slightly offset so we can check
    // the velocity

    auto cell = std::make_shared<ChipDB::Cell>("cell");
    cell->m_size = ChipDB::Coord64{500,500};

    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

    for(int32_t ox=-1; ox<=1; ox++)
    {
        for(int32_t oy=-1; oy<=1; oy++)
        {
            std::stringstream ss;
            ss << "ins_x" << ox << "_y" << oy;

            auto instance = std::make_shared<ChipDB::Instance>(ss.str(), cell);
            instance->m_pos  = ChipDB::Coord64{xsize/2 + ox*200 - cell->m_size.m_x/2, ysize/2 + oy*200 - cell->m_size.m_y/2};
            instance->m_placementInfo = ChipDB::PlacementInfo::PLACED;

            doLog(LOG_VERBOSE,"  instance %s at %d,%d\n", ss.str().c_str(), instance->m_pos.m_x, instance->m_pos.m_y);

            netlist.m_instances.add(instance);
        }
    }

    std::unique_ptr<LunaCore::QPlacer::DensityBitmap> bm(LunaCore::QPlacer::createDensityBitmap(&netlist, &region, bitmapCellSize, bitmapCellSize));
    BOOST_CHECK(bm != nullptr);
    BOOST_CHECK(bm->width() == 5);
    BOOST_CHECK(bm->height() == 5);
    
    LunaCore::QPlacer::setMinimalDensities(bm.get(), 1.0f);

    // check that the average density is 1
    double totalDensity = 0;
    for(size_t x=0; x<bm->width(); x++)
    {
        for(size_t y=0; y<bm->height(); y++)
        {
            totalDensity += bm->at(x,y);
        }
    }

    BOOST_TEST(bm->at(2,2) == 9*0.25);
    BOOST_TEST(bm->at(2,1) == 0.958333, tt::tolerance(0.1));
    BOOST_TEST(bm->at(2,3) == 0.958333, tt::tolerance(0.1));

    doLog(LOG_VERBOSE, "  bm(2,2) = %f\n", bm->at(2,2));
    doLog(LOG_VERBOSE, "  bm(2,1) = %f\n", bm->at(2,1));
    doLog(LOG_VERBOSE, "  bm(2,3) = %f\n", bm->at(2,3));

    BOOST_CHECK(totalDensity > 25.0f);
    BOOST_CHECK(totalDensity < 26.0f);

    LunaCore::QPlacer::VelocityBitmap vm(bm->width(), bm->height());
    LunaCore::QPlacer::calcVelocityBitmap(bm.get(), &vm);

    // check velocity below the center
    // it should have a vector pointing downwards
    BOOST_TEST((vm.at(2,1).m_dx == 0));
    BOOST_TEST((vm.at(2,1).m_dy < 0));

    doLog(LOG_VERBOSE, "  vm(2,1) = %f,%f\n", vm.at(2,1).m_dx, vm.at(2,1).m_dy);
    doLog(LOG_VERBOSE, "  vm(2,2) = %f,%f\n", vm.at(2,2).m_dx, vm.at(2,2).m_dy);
    doLog(LOG_VERBOSE, "  vm(2,3) = %f,%f\n", vm.at(2,3).m_dx, vm.at(2,3).m_dy);

    doLog(LOG_VERBOSE, "  vm(1,2) = %f,%f\n", vm.at(1,2).m_dx, vm.at(1,2).m_dy);
    doLog(LOG_VERBOSE, "  vm(3,2) = %f,%f\n", vm.at(3,2).m_dx, vm.at(3,2).m_dy);

    // check the velocity interpolation
    // due to point symmetry, the velocity in the middle
    // should be zero
    auto middleVelocity = LunaCore::QPlacer::interpolateVelocity(&vm, bitmapCellSize, bitmapCellSize, ChipDB::Coord64{xsize/2, ysize/2});

    BOOST_CHECK(middleVelocity.m_dx == 0);
    BOOST_CHECK(middleVelocity.m_dy == 0);

    // check on the boundary of middle cell and one to the left
    // should be half the velocity between left and middle cell
    auto leftVelocity = LunaCore::QPlacer::interpolateVelocity(&vm, bitmapCellSize, bitmapCellSize, ChipDB::Coord64{xsize/2 - bitmapCellSize/2, ysize/2});

    BOOST_CHECK(leftVelocity.m_dx == (vm.at(1,2).m_dx / 2.0));
    BOOST_CHECK(leftVelocity.m_dy == (vm.at(1,2).m_dy / 2.0));

    auto downVelocity = LunaCore::QPlacer::interpolateVelocity(&vm, bitmapCellSize, bitmapCellSize, ChipDB::Coord64{xsize/2, ysize/2 - bitmapCellSize/2});

    BOOST_CHECK(downVelocity.m_dx == (vm.at(2,1).m_dx / 2.0));
    BOOST_CHECK(downVelocity.m_dy == (vm.at(2,1).m_dy / 2.0));

    // dump the old positions
    doLog(LOG_VERBOSE, "  Old instance positions: \n");
    size_t idx = 0;
    for(auto insKeyObjPair : netlist.m_instances)
    {
        const auto insCenter = insKeyObjPair->getCenter();
        doLog(LOG_VERBOSE, "    ins %d -> (%d,%d)\n", idx, insCenter.m_x, insCenter.m_y);
        idx++;
    }

    // move the cells
    LunaCore::QPlacer::updateMovableInstances(&netlist, &region, &vm, bitmapCellSize, bitmapCellSize);

    // dump the new positions
    doLog(LOG_VERBOSE, "  New instance positions: \n");
    idx = 0;
    for(auto insKeyObjPair : netlist.m_instances)
    {
        const auto insCenter = insKeyObjPair->getCenter();
        doLog(LOG_VERBOSE, "    ins %d -> (%d,%d)\n", idx, insCenter.m_x, insCenter.m_y);
        idx++;
    }

    setLogLevel(ll);
}


BOOST_AUTO_TEST_SUITE_END()