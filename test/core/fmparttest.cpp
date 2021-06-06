#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FMPartTest)


BOOST_AUTO_TEST_CASE(can_partition)
{
    std::cout << "--== FMPart test ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(&design, verilogfile);

    auto mod = design.m_moduleLib.lookup("nerv");

    // nerv fits in approx 650x650 um    
    LunaCore::FMPart partitioner;
    partitioner.m_part1.m_region = {{0,0}, {650000/2, 650000}};             // left partition
    partitioner.m_part2.m_region = {{650000/2, 650000}, {650000, 650000}};  // right partition

    // allocate pin instances
    int64_t top_x    = 0;
    int64_t bottom_x = 0;
    for(auto ins : mod->m_netlist.m_instances)
    {
        if (ins->m_insType == ChipDB::Instance::INS_PIN)
        {
            auto pinInfo = ins->getPinInfo(0);
            if (pinInfo->isInput())
            {
                ins->m_pos = {top_x, 650000};
                ins->m_placementInfo = ChipDB::PLACEMENT_PLACEDANDFIXED;
                top_x += 5000;
            }
            else
            {
                ins->m_pos = {bottom_x, 0};
                ins->m_placementInfo = ChipDB::PLACEMENT_PLACEDANDFIXED;
                bottom_x += 5000;
            }
        }
    }

    partitioner.init(&mod->m_netlist);

    std::cout << "  Pins:\n";
    for(auto ins : mod->m_netlist.m_instances)
    {
        if (ins->m_insType == ChipDB::Instance::INS_PIN)
        {
            std::cout << "    pin " << ins->m_name << " " << toString(ins->getPinInfo(0)->m_iotype);
            std::cout << "\tpos " << ins->m_pos << "    partition: " << partitioner.m_nodes[ins->m_flags].m_partitionId << "\n";
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
