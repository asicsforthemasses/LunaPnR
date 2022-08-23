// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <chrono>
#include <stdexcept>
#include <stdio.h>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FMPartTest)

#if 0
//FIXME: refactor this mess!

class FMPartTestHelper : public LunaCore::Partitioner::FMPart
{
public:
    void addNodeToBucket(LunaCore::Partitioner::NodeId nodeId)
    {
        LunaCore::Partitioner::FMPart::addNodeToBucket(nodeId);
    }

    void removeNodeFromBucket(LunaCore::Partitioner::NodeId nodeId)
    {
        LunaCore::Partitioner::FMPart::removeNodeFromBucket(nodeId);
    }  

    bool init(ChipDB::Netlist *nl)
    {
        return LunaCore::Partitioner::FMPart::init(nl);
    }

    int64_t cycle()
    {
        return LunaCore::Partitioner::FMPart::cycle();
    }

    int64_t calculateCost() const
    {
        return LunaCore::Partitioner::FMPart::calculateCost();
    }

    using PartitionId = LunaCore::Partitioner::PartitionId;

    /** save the number of connected nodes */
    void saveConnectionCount()
    {
        m_netConnectionCount.resize(m_nets.size());
        size_t netIdx = 0;
        for(auto const& net : m_nets)
        {
            m_netConnectionCount[netIdx] = net.m_nodesInPartition[0] + net.m_nodesInPartition[1];
            assert(m_netConnectionCount[netIdx] == net.m_nodes.size());
            netIdx++;
        }
    }

    /** sanity check to make sure the number of nodes in a net is the same
     *  as the number of nodes in each partition connected to that net.
    */
    bool compareConnectionCount()
    {
        if (m_netConnectionCount.size() != m_nets.size())
        {
            std::cout << "m_netConnectionCount.size() == m_nets.size() failed\n";
            return false;
        }

        size_t netIdx = 0;
        bool   noError = true;
        for(auto const& net : m_nets)
        {
            auto newTotal = net.m_nodesInPartition[0] + net.m_nodesInPartition[1];
            auto oldTotal = m_netConnectionCount[netIdx];
            if (oldTotal != newTotal)
            {
                if (noError)
                    std::cout << "total nodes in partitions is not invariant between FM cycles!\n";

                std::cout << "  netId: " << netIdx << " before: " << oldTotal << " after: " << newTotal << "\n";
                noError = false;
            }
            assert(m_netConnectionCount[netIdx] == net.m_nodes.size());
            netIdx++;
        }

        return noError;
    }

    /** write the node partitioning to an internal vector so we 
     *  can call 'countNodesMoved' later.
    */
    void saveOriginalNodePartitioning()
    {
        m_originalNodePartition.resize(m_nodes.size());
        size_t nodeIdx = 0;
        for(auto const& node : m_nodes)
        {
            m_originalNodePartition[nodeIdx] = node.m_partitionId;
            nodeIdx++;
        }
    }

    /** returns the number of nodes that have moved partitions */
    ssize_t countNodesMoved()
    {
        if (m_originalNodePartition.size() != m_nodes.size())
        {
            std::cout << "m_originalNodePartition.size) != m_nodes.size() failed\n";
            return -1; 
        }

        ssize_t count = 0;
        size_t nodeIdx = 0;
        for(auto const& node : m_nodes)
        {
            if (m_originalNodePartition[nodeIdx] != node.m_partitionId)
            {
                count++;
            }

            nodeIdx++;
        }

        return count;
    }

    void reportPartitions(std::ostream &os)
    {
        os << "  Partition report:\n";
        size_t partCount = 0;
        for(auto& partition : m_partitions)
        {
            os << "    Partition " << partCount << "\n";
            for(auto node : partition)
            {
                os << "    ID: " << node->m_self << "    gain = " << node->m_gain << "  " << (node->isFixed() ? "LOCKED!" : "") << "  " << (node->isFixed() ? "FIXED!" : "") << "\n";
                BOOST_CHECK(node->isLocked() == node->isFixed());
            }
            partCount++;
        }

        os << "\n\n";
        reportPartitionSizes(os);
        os << "\n\n";
    }

    void reportPartitionSizes(std::ostream &os)
    {
        std::vector<int64_t> partitionWeight;
        partitionWeight.resize(m_partitions.size());

        for(auto const& node : m_nodes)
        {
            partitionWeight.at(node.m_partitionId) += node.m_weight;
        }

        for(size_t idx=0; idx < partitionWeight.size(); idx++)
        {
            os << " Partition[" << idx << "] weight = " << partitionWeight[idx] << "\n";
        }
    }

    size_t calcNumberOfCutNets() const
    {
        size_t numberOfCutNets = 0;
        for(auto const& net : m_nets)
        {
            if (net.m_nodes.size() == 0)
                continue;

            auto const& firstNode = m_nodes.at(net.m_nodes.at(0));
            PartitionId lastId = firstNode.m_partitionId;

            for(auto nodeId : net.m_nodes)
            {
                auto const& node = m_nodes.at(nodeId);
                if (node.m_partitionId != lastId)
                {
                    numberOfCutNets++;
                    continue;   // on to the next net!
                }
                lastId = node.m_partitionId;
            }
        }
        return numberOfCutNets;
    }

    size_t reportNumberOfCutNets(std::ostream &os)
    {
        size_t numberOfCutNets = calcNumberOfCutNets();
        os << "  Number of cut nets: " << numberOfCutNets << "\n";
        return numberOfCutNets;
    }

    int64_t distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos)
    {
        return LunaCore::Partitioner::FMPart::distanceToPartition(part, pos);
    }

    std::vector<size_t>         m_netConnectionCount;
    std::vector<PartitionId>    m_originalNodePartition;
};

BOOST_AUTO_TEST_CASE(test_distance_to_partition)
{
    std::cout << "--== FMPart (distance to partition) test ==--\n";

    FMPartTestHelper helper;

    std::vector<LunaCore::Partitioner::Node> nodes;
    FMPartTestHelper::Partition part1(nodes);
    FMPartTestHelper::Partition part2(nodes);
    part1.m_region = {{0,0},{10000,10000}};
    part2.m_region = {{10000,0},{20000,10000}};

    BOOST_CHECK(helper.distanceToPartition(part1, ChipDB::Coord64(0,0)) == 0);
    BOOST_CHECK(helper.distanceToPartition(part2, ChipDB::Coord64(0,0)) == 10000);

    BOOST_CHECK(helper.distanceToPartition(part1, ChipDB::Coord64(10000,0)) == 0);
    BOOST_CHECK(helper.distanceToPartition(part2, ChipDB::Coord64(10000,0)) == 0);
    
    BOOST_CHECK(helper.distanceToPartition(part1, ChipDB::Coord64(20000,0)) == 10000);
    BOOST_CHECK(helper.distanceToPartition(part2, ChipDB::Coord64(20000,0)) == 0);    

    BOOST_CHECK(helper.distanceToPartition(part1, ChipDB::Coord64(20000,20000)) == 20000);
    BOOST_CHECK(helper.distanceToPartition(part2, ChipDB::Coord64(20000,20000)) == 10000);  
}

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

    // add all nodes
    for(auto& node : helper.m_nodes)
    {
        helper.addNodeToBucket(node.m_self);
    }

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
    helper.removeNodeFromBucket(1);

    // check that node 1 is now unlinked
    BOOST_CHECK(helper.m_nodes[1].isLinked() == false);

    // check that node 0 is now the new head of the partition bucket
    BOOST_CHECK(helper.m_partitions[0].m_buckets[0 /* gain */] == 0);

    // check that node 0 is also unlinked (because it's the only node in the bucket)
    BOOST_CHECK(helper.m_nodes[0].isLinked() == false);

    // remove the last node from the partition 0 bucket
    // and check that the bucket no longer exists
    helper.removeNodeFromBucket(0);
    BOOST_CHECK(!helper.m_partitions[0].hasBucket(0));

    // remove the last node from the partition 1 bucket
    // and check that the bucket no longer exists
    helper.removeNodeFromBucket(2);
    BOOST_CHECK(!helper.m_partitions[1].hasBucket(0));

    // ==== check the partition iterator ====
    helper.m_nodes.resize(5);
    helper.m_nodes[3].reset(3);
    helper.m_nodes[4].reset(4);    
    helper.m_nodes[3].m_partitionId = 0;
    helper.m_nodes[4].m_partitionId = 0;
    helper.m_nodes[3].m_gain = 1;
    helper.m_nodes[4].m_gain = 1;

    // add all nodes
    for(auto& node : helper.m_nodes)
    {
        helper.addNodeToBucket(node.m_self);
    }

    size_t count = 0;
    std::cout << "  Partition 0 contains the following nodes:\n";

    LunaCore::Partitioner::GainType lastGain = 100e6;

    auto iter = helper.m_partitions[0].begin();
    while(iter != helper.m_partitions[0].end())
    {
        std::cout << "  Id:" << iter->m_self << "   gain:"<< iter->m_gain << "\n";

        // make sure the previous gain is higher or equal to the current gain
        BOOST_CHECK(iter->m_gain <= lastGain);
        lastGain = iter->m_gain;

        count++;
        ++iter;
    }
    BOOST_CHECK(count == 4);

    // check if for(..) is supported by the iterator
    count = 0;
    for(auto node : helper.m_partitions[0])
    {
        count++;
    }
    BOOST_CHECK(count == 4);
}

BOOST_AUTO_TEST_CASE(can_partition_adder2)
{
    uint32_t oldLogLevel = getLogLevel();

    setLogLevel(LogType::VERBOSE);

    std::cout << "--== FMPart (partitioning ADDER2) test ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/adder2.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(&design, verilogfile);

    auto mod = design.m_moduleLib.lookup("adder2");

    FMPartTestHelper partitioner;
    partitioner.m_partitions[0].m_region = {{0,0}, {65000/2, 65000}};       // left partition
    partitioner.m_partitions[1].m_region = {{65000/2, 0}, {65000, 65000}};  // right partition

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
                ins->m_pos = {65000, right_y};
                ins->m_placementInfo = ChipDB::PLACEMENT_PLACEDANDFIXED;
                right_y += 5000;
            }
        }
    }

    partitioner.init(&mod->m_netlist);

    // write partition out as a dot file
    std::ofstream dotfile2("test/files/results/adder2_before.dot");
    if (dotfile2.good())
    {        
        partitioner.exportToDot(dotfile2);
        dotfile2.close();
    }

    partitioner.saveConnectionCount();
    partitioner.saveOriginalNodePartitioning();
    
    for(size_t i=0; i<7; i++)
    {
        auto cost = partitioner.cycle();
        ssize_t nodesMoved = partitioner.countNodesMoved();
        BOOST_CHECK(nodesMoved >= 0);

        ssize_t cutNets = partitioner.calcNumberOfCutNets();

        std::cout << "    cycle " << i << " nodes moved = " << nodesMoved << "  Nets cut: " << cutNets << "  Cost: " << cost << "\n";
        BOOST_CHECK(partitioner.compareConnectionCount());
    }

    // write partition out as a dot file
    std::ofstream dotfile("test/files/results/adder2_after.dot");
    if (dotfile.good())
    {        
        partitioner.exportToDot(dotfile);
        dotfile.close();
    }

    setLogLevel(oldLogLevel);
}

BOOST_AUTO_TEST_CASE(can_partition_multiplier)
{
    std::cout << "--== FMPart (partitioning MULTIPLIER) test ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/multiplier.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(&design, verilogfile);

    auto mod = design.m_moduleLib.lookup("multiplier");

    FMPartTestHelper partitioner;
    partitioner.m_partitions[0].m_region = {{0,0}, {65000/2, 65000}};             // left partition
    partitioner.m_partitions[1].m_region = {{65000/2, 0}, {65000, 65000}};  // right partition

    // allocate pin instances
    int64_t left_y    = 0;
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
                ins->m_pos = {65000, right_y};
                ins->m_placementInfo = ChipDB::PLACEMENT_PLACEDANDFIXED;
                right_y += 5000;
            }
        }
    }

    partitioner.init(&mod->m_netlist);

    // write partition out as a dot file
    std::ofstream dotfile2("test/files/results/multiplier_before.dot");
    if (dotfile2.good())
    {        
        partitioner.exportToDot(dotfile2);
        dotfile2.close();
    }

    partitioner.saveConnectionCount();
    partitioner.saveOriginalNodePartitioning();
    
    for(size_t i=0; i<7; i++)
    {
        auto cost = partitioner.cycle();
        ssize_t nodesMoved = partitioner.countNodesMoved();
        BOOST_CHECK(nodesMoved >= 0);

        ssize_t cutNets = partitioner.calcNumberOfCutNets();

        std::cout << "    cycle " << i << " nodes moved = " << nodesMoved << "  Nets cut: " << cutNets << "  Cost: " << cost << "\n";
        BOOST_CHECK(partitioner.compareConnectionCount());
    }

    // write partition out as a dot file
    std::ofstream dotfile("test/files/results/multiplier_after.dot");
    if (dotfile.good())
    {        
        partitioner.exportToDot(dotfile);
        dotfile.close();
    }

}

BOOST_AUTO_TEST_CASE(can_partition_nerv)
{
    std::cout << "--== FMPart (partitioning NERV CPU) test ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(&design, verilogfile);

    auto mod = design.m_moduleLib.lookup("nerv");

    // nerv fits in approx 650x650 um    
    FMPartTestHelper partitioner;
    partitioner.m_partitions[0].m_region = {{0,0}, {650000/2, 650000}};             // left partition
    partitioner.m_partitions[1].m_region = {{650000/2, 0}, {650000, 650000}};  // right partition

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

    partitioner.init(&mod->m_netlist);

    // check that no nodes appear more than once
    std::vector<int32_t> nodeCount;
    nodeCount.resize(partitioner.m_nodes.size());

    for(auto partition : partitioner.m_partitions)
    {
        for(auto nodePtr : partition)
        {
            BOOST_CHECK(nodeCount.at(nodePtr->m_self)==0);
            nodeCount.at(nodePtr->m_self)++;
        }
    }
    nodeCount.clear();

    // check that there are nodes that are fixed
    size_t numberOfFixedNodes = 0;
    for(auto node : partitioner.m_nodes)
    {
        if (node.isFixed())
            numberOfFixedNodes++;
    }
    BOOST_CHECK(numberOfFixedNodes > 0);
    std::cout << "  There are " << numberOfFixedNodes << " fixed nodes\n";

    // check that node 'self' ids are consistent
    ssize_t testIdx = 0;
    for(auto const& node : partitioner.m_nodes)
    {
        BOOST_CHECK(node.m_self == testIdx);
        testIdx++;
    }

    // check that all nodes appear in the correct bucket
    for(auto& partition : partitioner.m_partitions)
    {
        auto iter = partition.begin();
        while(iter != partition.end())
        {
            BOOST_CHECK(iter->m_gain == iter.getBucketGain());
            ++iter;
        }
    }

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

    // go thought the partition with the bucket iterator
    partitioner.reportPartitions(ofile);
    std::cout << "  number of cut nets: " << partitioner.reportNumberOfCutNets(ofile) << "\n";
    std::cout << "  total cost        : " << partitioner.calculateCost() << "\n";

    // perform a few cycles of the FM algo
    std::cout << "  starting FM cycles..\n";
    ofile << "  starting FM cycles..\n";
    partitioner.saveConnectionCount();
    partitioner.saveOriginalNodePartitioning();
    for(size_t i=0; i<2; i++)
    {
        auto cost = partitioner.cycle();
        ssize_t nodesMoved = partitioner.countNodesMoved();
        BOOST_CHECK(nodesMoved >= 0);

        //ssize_t cutNets = partitioner.calcNumberOfCutNets();

        std::cout << "    cycle " << i << " nodes moved = " << nodesMoved << "  Cost: " << cost << "\n";
        ofile     << "    cycle " << i << " nodes moved = " << nodesMoved << "  Cost: " << cost << "\n";
        BOOST_CHECK(partitioner.compareConnectionCount());
    }
    std::cout << "  end FM\n";
    ofile << "  end FM\n";    
    
    // go thought the partition with the bucket iterator
    partitioner.reportPartitions(ofile);
    std::cout << "  number of cut nets: " << partitioner.reportNumberOfCutNets(ofile) << "\n";

    std::time_t result = std::time(nullptr);
    ofile << "Produced: " << std::ctime(&result);
}
#endif

BOOST_AUTO_TEST_CASE(can_partition_multiplier)
{
    std::cout << "--== FMPart (partitioning MULTIPLIER) test ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/multiplier.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(design, verilogfile);

    auto modKeyObjPair = design.m_moduleLib->lookupModule("multiplier");

    LunaCore::Partitioner::FMPart partitioner;
    LunaCore::Partitioner::FMContainer container;
    container.m_region = {{0,0}, {65000, 65000}};

    // allocate pin instances
    int64_t left_y    = 0;
    int64_t right_y = 0;
    for(auto insKeyObjPair : modKeyObjPair->m_netlist->m_instances)
    {
        if (insKeyObjPair->m_insType == ChipDB::InstanceType::PIN)
        {
            auto const& pin = insKeyObjPair->getPin(0);

            BOOST_CHECK(pin.isValid());

            if (pin.m_pinInfo->isInput())
            {
                insKeyObjPair->m_pos = {0, left_y};
                insKeyObjPair->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
                left_y += 5000;
            }
            else
            {
                insKeyObjPair->m_pos = {65000, right_y};
                insKeyObjPair->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
                right_y += 5000;
            }
        }
    }

    auto prevLogLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::VERBOSE);

    partitioner.doPartitioning(modKeyObjPair->m_netlist.get(), container);

    // write partition out as a dot file
    std::ofstream dotfile("test/files/results/multiplier_after.dot");
    if (dotfile.good())
    {        
        partitioner.exportToDot(dotfile, container);
        dotfile.close();
    }
    Logging::setLogLevel(prevLogLevel);
}


BOOST_AUTO_TEST_CASE(can_partition_nerv_concise)
{
    std::cout << "--== FMPart (partitioning NERV CPU) test ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/nerv_tsmc018.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(design, verilogfile);

    auto modKeyObjPair = design.m_moduleLib->lookupModule("nerv");

    // nerv fits in approx 650x650 um    
    LunaCore::Partitioner::FMPart partitioner;
    LunaCore::Partitioner::FMContainer container;
    container.m_region = {{0,0}, {650000, 650000}};

    // check that netlist exists
    BOOST_CHECK(modKeyObjPair->m_netlist);

    // allocate pin instances
    int64_t left_y  = 0;
    int64_t right_y = 0;
    for(auto insKeyObjPair : modKeyObjPair->m_netlist->m_instances)
    {
        if (insKeyObjPair->m_insType == ChipDB::InstanceType::PIN)
        {
            auto const& pin = insKeyObjPair->getPin(0);
            BOOST_CHECK(pin.isValid());

            if (pin.m_pinInfo->isInput())
            {
                insKeyObjPair->m_pos = {0, left_y};
                insKeyObjPair->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
                left_y += 5000;
            }
            else
            {
                insKeyObjPair->m_pos = {650000, right_y};
                insKeyObjPair->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
                right_y += 5000;
            }
        }
    }

    auto prevLogLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::VERBOSE);

    if (!partitioner.doPartitioning(modKeyObjPair->m_netlist.get(), container))
    {
        Logging::doLog(Logging::LogType::ERROR, "Partitioning failed!\n");
        BOOST_CHECK(false);
    }

    // check that the clock net is huge
    auto clknetKeyObjPair = modKeyObjPair->m_netlist->m_nets.at("clock");
    BOOST_ASSERT(clknetKeyObjPair.isValid());
    
    //FIXME: objects no longer have an ID
    //auto cnet = container.m_nets.at(clknetKeyObjPair->m_id);
    //BOOST_ASSERT(cnet.m_nodes.size() > 25);

    setLogLevel(prevLogLevel);
}



BOOST_AUTO_TEST_SUITE_END()
