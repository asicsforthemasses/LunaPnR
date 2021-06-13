#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FMPartTest)

class FMPartTestHelper : public LunaCore::Partitioner::FMPart
{
public:

    bool addNodeToPartitionBucket(const LunaCore::Partitioner::NodeId nodeId)
    {
        return LunaCore::Partitioner::FMPart::addNodeToPartitionBucket(nodeId);
    }

    bool removeNodeFromPartitionBucket(const LunaCore::Partitioner::NodeId nodeId)
    {
        return LunaCore::Partitioner::FMPart::removeNodeFromPartitionBucket(nodeId);
    }
};

BOOST_AUTO_TEST_CASE(test_buckets)
{
    std::cout << "--== FMPart (buckets) test ==--\n";

    FMPartTestHelper helper;

    BOOST_CHECK(helper.m_partitions.size() == 2);

    helper.m_nodes.resize(3);
    helper.m_nodes[0].reset(0);
    helper.m_nodes[1].reset(1);
    helper.m_nodes[2].reset(2);

    BOOST_CHECK(helper.m_nodes[0].isLinked() == false);
    BOOST_CHECK(helper.m_nodes[1].isLinked() == false);
    BOOST_CHECK(helper.m_nodes[2].isLinked() == false);

    helper.m_nodes[0].m_partitionId = 0;
    helper.m_nodes[1].m_partitionId = 0;
    helper.m_nodes[2].m_partitionId = 1;

    // check that buckets for gain = 0 do not exist
    BOOST_CHECK(!helper.m_partitions[0].hasBucket(0));
    BOOST_CHECK(!helper.m_partitions[1].hasBucket(0));

    helper.addNodeToPartitionBucket(0);
    helper.addNodeToPartitionBucket(1);
    helper.addNodeToPartitionBucket(2);

    // check that buckets for gain = 0 exist
    BOOST_CHECK(helper.m_partitions[0].hasBucket(0));
    BOOST_CHECK(helper.m_partitions[1].hasBucket(0));
    
    // node 1 should be at the front of the only partition 0 bucket
    BOOST_CHECK(helper.m_partitions[0].m_buckets[0 /* gain */] == 1);
    
    // node 2 should be at the front of the only partition 1 bucket for gain = 0
    BOOST_CHECK(helper.m_partitions[1].m_buckets[0 /* gain */] == 2);

    // check that node 1 is followed by node 2
    BOOST_CHECK(helper.m_nodes[1].m_prev == -1); // node 1 is head so no previous node exists
    BOOST_CHECK(helper.m_nodes[1].m_next == 0);  // node 0 comes after node 1
    BOOST_CHECK(helper.m_nodes[0].m_prev == 1);  // node 1 comes before node 0
    BOOST_CHECK(helper.m_nodes[0].m_next == -1); // there is no node after node 0 in the bucket

    // check that node 2 is the only node in partition 2 bucket for gain = 0
    BOOST_CHECK(helper.m_nodes[2].m_next == -1); // node 2 is alone..
    BOOST_CHECK(helper.m_nodes[2].m_prev == -1); // node 2 is alone..

    // now, remove node 1 (head node) from the partition bucket
    helper.removeNodeFromPartitionBucket(1);

    // check that node 1 is now unlinked
    BOOST_CHECK(helper.m_nodes[1].isLinked() == false);

    // check that node 0 is now the new head of the partition bucket
    BOOST_CHECK(helper.m_partitions[0].m_buckets[0 /* gain */] == 0);

    // check that node 0 is also unlinked (because it's the only node in the bucket)
    BOOST_CHECK(helper.m_nodes[0].isLinked() == false);

    // remove the last node from the partition 0 bucket
    // and check that the bucket no longer exists
    helper.removeNodeFromPartitionBucket(0);
    BOOST_CHECK(!helper.m_partitions[0].hasBucket(0));

    // remove the last node from the partition 1 bucket
    // and check that the bucket no longer exists
    helper.removeNodeFromPartitionBucket(2);
    BOOST_CHECK(!helper.m_partitions[1].hasBucket(0));

    // ==== check the partition iterator ====
    helper.m_nodes.resize(5);
    helper.m_nodes[3].reset(3);
    helper.m_nodes[4].reset(4);    
    helper.m_nodes[3].m_partitionId = 0;
    helper.m_nodes[4].m_partitionId = 0;
    helper.m_nodes[3].m_gain = 1;
    helper.m_nodes[4].m_gain = 1;

    helper.addNodeToPartitionBucket(0);
    helper.addNodeToPartitionBucket(1);
    helper.addNodeToPartitionBucket(2);
    helper.addNodeToPartitionBucket(3);
    helper.addNodeToPartitionBucket(4);

    size_t count = 0;
    std::cout << "  Partition 0 contains the following nodes:\n";
    auto iter = helper.m_partitions[0].begin();
    while(iter != helper.m_partitions[0].end())
    {
        std::cout << "  Id:" << iter->m_self << "   gain:"<< iter->m_gain << "\n";
        count++;
        ++iter;
    }
    BOOST_CHECK(count == 4);
}

BOOST_AUTO_TEST_CASE(can_partition)
{
    std::cout << "--== FMPart (partitioning) test ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(&design, verilogfile);

    auto mod = design.m_moduleLib.lookup("nerv");

    // nerv fits in approx 650x650 um    
    LunaCore::Partitioner::FMPart partitioner;
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
