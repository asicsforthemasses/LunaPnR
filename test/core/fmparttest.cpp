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

//FIXME: refactor this mess!

class FMPartTestHelper : public LunaCore::Partitioner::FMPart
{
public:
    void addNode(LunaCore::Partitioner::NodeId nodeId)
    {
        LunaCore::Partitioner::FMPart::addNode(nodeId);
    }

    void removeNode(LunaCore::Partitioner::NodeId nodeId)
    {
        LunaCore::Partitioner::FMPart::removeNode(nodeId);
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
                os << "    ID: " << node->m_self << "    gain = " << node->m_gain << "  " << (node->m_locked ? "LOCKED!" : "") << "  " << (node->m_fixed ? "FIXED!" : "") << "\n";
                BOOST_CHECK(node->m_locked == node->m_fixed);
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

    std::vector<size_t>         m_netConnectionCount;
    std::vector<PartitionId>    m_originalNodePartition;
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

    // add all nodes
    for(auto& node : helper.m_nodes)
    {
        helper.addNode(node.m_self);
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
    helper.removeNode(1);

    // check that node 1 is now unlinked
    BOOST_CHECK(helper.m_nodes[1].isLinked() == false);

    // check that node 0 is now the new head of the partition bucket
    BOOST_CHECK(helper.m_partitions[0].m_buckets[0 /* gain */] == 0);

    // check that node 0 is also unlinked (because it's the only node in the bucket)
    BOOST_CHECK(helper.m_nodes[0].isLinked() == false);

    // remove the last node from the partition 0 bucket
    // and check that the bucket no longer exists
    helper.removeNode(0);
    BOOST_CHECK(!helper.m_partitions[0].hasBucket(0));

    // remove the last node from the partition 1 bucket
    // and check that the bucket no longer exists
    helper.removeNode(2);
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
        helper.addNode(node.m_self);
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
    FMPartTestHelper partitioner;
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
        if (node.m_fixed)
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

    // perform a few cycles of the FM algo
    std::cout << "  starting FM cycles..\n";
    ofile << "  starting FM cycles..\n";
    partitioner.saveConnectionCount();
    partitioner.saveOriginalNodePartitioning();
    for(size_t i=0; i<20; i++)
    {
        auto gain = partitioner.cycle();
        ssize_t nodesMoved = partitioner.countNodesMoved();
        BOOST_CHECK(nodesMoved >= 0);

        ssize_t cutNets = partitioner.calcNumberOfCutNets();

        std::cout << "    cycle " << i << " gain = " << gain << " nodes moved = " << nodesMoved << "  Nets cut: " << cutNets << "\n";
        ofile     << "    cycle " << i << " gain = " << gain << " nodes moved = " << nodesMoved << "  Nets cut: " << cutNets << "\n";
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

BOOST_AUTO_TEST_SUITE_END()
