#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
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
    partitioner.m_partitions[0].m_region = {{0,0}, {650000/2, 650000}};             // left partition
    partitioner.m_partitions[1].m_region = {{650000/2, 650000}, {650000, 650000}};  // right partition

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

    auto ofile = std::ofstream("test/files/results/partitioner.txt");

    ofile << "  Pins:\n";
    for(auto ins : mod->m_netlist.m_instances)
    {
        if (ins->m_insType == ChipDB::Instance::INS_PIN)
        {
            ofile << "    pin " << ins->m_name << " " << toString(ins->getPinInfo(0)->m_iotype);
            ofile << "\tpos " << ins->m_pos << "    partition: " << partitioner.m_nodes[ins->m_flags].m_partitionId << "\n";
        }
    }

    uint64_t widthP[2] = {0,0};   // total cell width for partitions
    ofile << "  Instances:\n";
    for(auto ins : mod->m_netlist.m_instances)
    {
        if (ins->m_insType == ChipDB::Instance::INS_CELL)
        {
            ofile << "    cell " << ins->m_name << " gain: " << partitioner.m_nodes[ins->m_flags].m_gain << " ";
            ofile << "\tid: " << partitioner.m_nodes[ins->m_flags].m_self;
            ofile << "   part: " << partitioner.m_nodes[ins->m_flags].m_partitionId;
            ofile << "   next: " << partitioner.m_nodes[ins->m_flags].m_next << "   prev: " <<  partitioner.m_nodes[ins->m_flags].m_prev << "\n";
            widthP[partitioner.m_nodes[ins->m_flags].m_partitionId] += ins->instanceSize().m_x;
        }
    }
    
    ofile << "\n\n";

    size_t partId = 0;
    for(auto const& part : partitioner.m_partitions)
    {
        ofile << "Partition " << partId << "\n";
        for(auto const& bucket : part.m_buckets)
        {
            ofile << "  Bucket gain = " << bucket.first << "   head node = " << bucket.second << "\n";
        }
        partId++;
    }

    ofile << "\n\n";
    ofile << "Total widths:\n";
    ofile << "  P0 : " << widthP[0] << " nm\n";
    ofile << "  P1 : " << widthP[1] << " nm\n";
    ofile << "\n\n";

    std::time_t result = std::time(nullptr);
    ofile << "Produced: " << std::ctime(&result);
    
    
}

BOOST_AUTO_TEST_SUITE_END()
