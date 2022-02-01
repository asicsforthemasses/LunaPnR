/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <stdio.h>
#include <algorithm>
#include <boost/test/unit_test.hpp>


#if 0

BOOST_AUTO_TEST_SUITE(FemtoRV32_quark_test)



/*

    Test added to conver assign statements and multi-dimensional array wires

*/

BOOST_AUTO_TEST_CASE(test_can_read_FemtoRV32_quark)
{
    std::cout << "--== FemtoRV32 Quark test ==--\n";

    uint32_t oldLogLevel = getLogLevel();

    setLogLevel(LOG_VERBOSE);

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/femtorv32_quark.v");
    BOOST_CHECK(verilogfile.good());

    BOOST_CHECK(ChipDB::Verilog::Reader::load(&design, verilogfile));

    auto mod = design.m_moduleLib.lookup("FemtoRV32");
    BOOST_CHECK(mod != nullptr);

    // femtorv32 fits in approx 650x650 um    
    LunaCore::Partitioner::FMPart partitioner;
    LunaCore::Partitioner::FMContainer container;
    container.m_region = {{0,0}, {650000, 650000}};

    // allocate pin instances
    int64_t left_y  = 0;
    int64_t right_y = 0;
    for(auto ins : mod->m_netlist.m_instances)
    {
        if (ins->m_insType == ChipDB::Instance::INS_PIN)
        {
            auto pinInfo = ins->getPinInfo(0);
            if (pinInfo->isInput())
            {
                ins->m_pos = {0, left_y};
                ins->m_placementInfo = ChipDB::PLACEMENT_PLACEDANDFIXED;
                left_y += 5000;                
            }
            else
            {
                ins->m_pos = {650000, right_y};
                ins->m_placementInfo = ChipDB::PLACEMENT_PLACEDANDFIXED;
                right_y += 5000;
            }
        }
    }

    partitioner.doPartitioning(&mod->m_netlist, container);

    setLogLevel(oldLogLevel);
}



BOOST_AUTO_TEST_SUITE_END()

#endif
