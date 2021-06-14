#include <algorithm>
#include "fmpart.h"
#include <assert.h>
#include "common/logging.h"
#include "netlist/instance.h"

using namespace LunaCore::Partitioner;

int64_t FMPart::distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos)
{
    auto dx = std::max(part.m_region.m_ll.m_x - pos.m_x, int64_t(0));
         dx = std::max(dx, pos.m_x - part.m_region.m_ur.m_x);

    auto dy = std::max(part.m_region.m_ll.m_y - pos.m_y, int64_t(0));
         dy = std::max(dy, pos.m_y - part.m_region.m_ur.m_y);

    return dx+dy;
}

bool FMPart::init(ChipDB::Netlist *nl)
{
    if (nl == nullptr)
        return false;

    m_nodes.resize(nl->m_instances.size());
    m_nets.resize(nl->m_nets.size());

    // use the flags of the instance and 
    // nets to number each entity uniquely

    ssize_t nodeIdx = 0;
    for(auto ins : nl->m_instances)
    {
        ins->m_flags = nodeIdx;

        /* use cell width as the weight, as the height of each cell is the same */
        m_nodes[nodeIdx].reset(nodeIdx);
        m_nodes[nodeIdx].m_weight = ins->instanceSize().m_x;
        
        // check if the instance has a fixed position.
        // if so, assign the instance/node to the closest
        // partition

        if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEMENT_PLACEDANDFIXED)
        {
            if (distanceToPartition(m_partitions[0], ins->m_pos) < distanceToPartition(m_partitions[1], ins->m_pos))
            {
                m_nodes[nodeIdx].m_partitionId = 0;
            }
            else
            {
                m_nodes[nodeIdx].m_partitionId = 1;
            }
            m_nodes[nodeIdx].setFixed();
        }
        else
        {
            // randomly assign a partition
            // FIXME: do some area / fill balancing
            //m_nodes[nodeIdx].m_locked = false;
            if (std::rand() > (RAND_MAX/2))
            {
                m_nodes[nodeIdx].m_partitionId = 1;
            }
            else
            {
                m_nodes[nodeIdx].m_partitionId = 0;
            }
        }

        nodeIdx++;
    }

    size_t netIdx = 0;
    for(auto net : nl->m_nets)
    {
        net->m_flags = netIdx;
        m_nets[netIdx].m_weight = 1;    // all nets are the same weight
        m_nets[netIdx].m_nodesInPartition[0] = 0;
        m_nets[netIdx].m_nodesInPartition[1] = 0;
        netIdx++;
    }

    // set the connected nets of each node
    // and add the node to each net
    for(auto ins : nl->m_instances)
    {
        for(ssize_t pinIndex=0; pinIndex < ins->getNumberOfPins(); ++pinIndex)
        {
            auto net = ins->getConnectedNet(pinIndex);
            if (net != nullptr)
            {
                const auto netIndex  = net->m_flags;
                const auto nodeIndex = ins->m_flags;

                auto& net  = m_nets.at(netIndex);
                auto& node = m_nodes.at(nodeIndex);

                // FIXME: we should check each node for duplicate nets!
                node.m_nets.push_back(netIndex);

                // FIXME: we should check each net for duplicate nodes!
                net.m_nodes.push_back(nodeIndex);

                net.m_nodesInPartition[node.m_partitionId]++;
            }
        }
    }

    // calculate gains per instance
    for(auto& node : m_nodes)
    {        
        calcNodeGain(node);
        if (!node.m_locked) 
            addNode(node.m_self);
    }

    return true;
}

void FMPart::calcNodeGain(Node &node)
{
    node.m_gain = 0;

    // don't calculate gain for unmovable nodes.
    if (node.m_locked)
    {
        return;
    }

    uint32_t fromPartitionIndex = 0;
    uint32_t toPartitionIndex   = 0;
    if (node.m_partitionId == 0)
    {
        toPartitionIndex   = 1;            
    }
    else if (node.m_partitionId == 1)
    {
        fromPartitionIndex = 1;
    }
    else
    {
        // error -> m_partitionId not correct init'd
    }

    for(auto netId : node.m_nets)
    {
        auto const& net = m_nets[netId];

        // the net is not critical when it has more than two nodes
        // in each partition. Non-critical nets cannot change the
        // gain of a node.

        if (net.m_nodesInPartition[fromPartitionIndex] == 1)
        {
            // moving the last node in a partition to 
            // the other side will uncut the net.
            node.m_gain++;
        }

        if (net.m_nodesInPartition[toPartitionIndex] == 0)
        {
            // moving a node from an uncut net to
            // another partition will cut the net.
            node.m_gain--;
        }            
    }
}

GainType FMPart::cycle()
{
    struct FreeListType
    {
        NodeId      m_nodeId;
        GainType    m_totalGain;
    };

    std::vector<FreeListType> freeNodes;

    freeNodes.resize(m_nodes.size());

    size_t      freeNodeIdx = 0;
    GainType    totalGain = 0;
    bool updated = false;
    do
    {
        updated = false;
        // for now, we try to balance each partition by selecting the largest
        // partition
        auto partitionId = (m_partitions[1].getTotalWeight() > m_partitions[0].getTotalWeight()) ? 1 : 0;

        // find the node with the most gain
        auto iter = m_partitions[partitionId].begin();
        if (iter != m_partitions[partitionId].end())
        {
            updated = true;
            auto nodeId = iter->m_self;
            totalGain += iter->m_gain;

            assert(!iter->m_locked);

            try{
                freeNodes.at(freeNodeIdx).m_nodeId    = nodeId;  // save the node ID in the free list
                freeNodes.at(freeNodeIdx).m_totalGain = totalGain;
            }
            catch(...)
            {
                while(1) {};
            }

            freeNodeIdx++;

            try
            {
                removeNode(nodeId);
            }
            catch(...)
            {
                while(1) {};
            }

            // update the gains of the connected nodes
            try
            {
                moveNodeAndUpdateNeighbours(nodeId);
            }
            catch(...)
            {
                while(1) {};
            }
        }

    } while(updated);

    // ****************************************************
    // ** find the largest value of the cummulative gain **
    // ****************************************************

    GainType maxTotalGain     = 0;
    ssize_t  maxTotalGainIdx  = -1;

    ssize_t itemIdx = 0;
    for(ssize_t idx=0; idx < freeNodeIdx; idx++)
    {
        auto& item = freeNodes.at(idx);

        if (item.m_totalGain > maxTotalGain)
        {
            maxTotalGain    = item.m_totalGain;
            maxTotalGainIdx = itemIdx;
        }
        itemIdx++;
    }

    try
    {
        // undo the movement of the nodes
        // that come after the maximum gain node in the free list
        for(ssize_t idx=maxTotalGainIdx+1; idx < freeNodeIdx; idx++)
        {
            auto& item = freeNodes.at(idx);            
            auto& node = m_nodes.at(item.m_nodeId);

            size_t fromPartition = 0;
            size_t toPartition   = 0;
            if (node.m_partitionId == 0)
            {
                toPartition = 1;
            }
            else
            {
                fromPartition = 1;
            }

            node.m_partitionId = toPartition;
            for(auto netId : node.m_nets)
            {
                m_nets[netId].m_nodesInPartition[fromPartition]--;
                m_nets[netId].m_nodesInPartition[toPartition]++;
            }
        }
    }
    catch(...)
    {
        while(1) {};
    }
    
    // add all the nodes in the free list back into their
    // partition buckets
    for(ssize_t idx=0; idx < freeNodeIdx; idx++)
    {
        auto& item = freeNodes.at(idx);
        auto& node = m_nodes.at(item.m_nodeId);
        
        assert(m_partitions[node.m_partitionId].hasNode(node.m_self) == false);

        calcNodeGain(node);
        addNode(item.m_nodeId);

        // unlock the node, if it is not a fixed node
        m_nodes.at(item.m_nodeId).m_locked = m_nodes.at(item.m_nodeId).m_fixed;
    }

    return maxTotalGain;
}

void FMPart::moveNodeAndUpdateNeighbours(NodeId nodeId)
{
    auto& node = m_nodes.at(nodeId);

    size_t fromPartitionIndex = 0;
    size_t toPartitionIndex   = 0;

    if (node.m_partitionId == 0)
    {
        toPartitionIndex   = 1;
    }
    else if (node.m_partitionId == 1)
    {
        fromPartitionIndex = 1;
    }
    else
    {
        // error -> m_partitionId not correct init'd
        throw std::out_of_range("Partition ID of out range");
    }

    node.m_locked = true;
    node.m_partitionId = toPartitionIndex;    // move node
    
    for(auto netId : node.m_nets)
    {
        auto& net = m_nets[netId];

        // check critical net before the move
        if (net.m_nodesInPartition[toPartitionIndex] == 0)
        {
            // increment the gains on all the free cells on the net
            for(auto netNodeId : net.m_nodes)
            {
                auto& netNode = m_nodes.at(netNodeId);
                if (!netNode.m_locked)
                {                                        
                    removeNode(netNodeId);
                    netNode.m_gain++;
                    addNode(netNodeId);
                }
            }
        }
        else if (net.m_nodesInPartition[toPartitionIndex] == 1)
        {
            // decrement the gain of the only T cell on the net
            // if it's free
            size_t count = 0;
            for(auto netNodeId : net.m_nodes)
            {
                auto& netNode = m_nodes.at(netNodeId);
                if ((netNode.m_partitionId == toPartitionIndex) &&
                    !netNode.m_locked)
                {
                    removeNode(netNodeId);
                    netNode.m_gain--;
                    addNode(netNodeId);
                    count++;
                }
            }

            // check: count should be 1 or 0
            if (count > 1)     
            {
                doLog(LOG_ERROR,"FMPart::moveNodeAndUpdateNeightbours internal error: count != 1\n");
            }
        }

        // update the net information
        net.m_nodesInPartition[toPartitionIndex]++;
        net.m_nodesInPartition[fromPartitionIndex]--;

        assert(net.m_nodesInPartition[toPartitionIndex] >= 0);
        assert(net.m_nodesInPartition[fromPartitionIndex] >= 0);

        // check critical net after the move
        if (net.m_nodesInPartition[fromPartitionIndex] == 0)
        {
            // decrement gains of all free cells on the net
            for(auto netNodeId : net.m_nodes)
            {
                auto& netNode = m_nodes.at(netNodeId);
                if (!netNode.m_locked)
                {                    
                    removeNode(netNodeId);
                    netNode.m_gain--;
                    addNode(netNodeId);
                }
            }
        }
        else if (net.m_nodesInPartition[fromPartitionIndex] == 1)
        {
            // increment gain of the only F cell on the net
            // if it's free
            size_t count = 0;
            for(auto netNodeId : net.m_nodes)
            {                
                auto& netNode = m_nodes.at(netNodeId);
                if ((netNode.m_partitionId == fromPartitionIndex) &&
                    !netNode.m_locked)
                {                    
                    removeNode(netNodeId);
                    netNode.m_gain++;
                    addNode(netNodeId);               
                }
            }    

            // check: count should be 1 or 0
            if (count > 1)
            {
                doLog(LOG_ERROR,"FMPart::moveNodeAndUpdateNeightbours internal error: count != 1\n");
            }                                      
        }
    }
}
