#include <cassert>
#include <limits>
#include <unordered_map>
#include <stack>
#include <algorithm>

#include "common/logging.h"
#include "netlist/instance.h"
#include "fmpart.h"

using namespace LunaCore::Partitioner;


void FMContainer::calcAndSetNodeGain(Node &node)
{
    node.m_gain = 0;

    // don't calculate gain for unmovable nodes.
    if (node.isLocked() || node.isFixed())
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
        auto const& net = m_nets.at(netId);

        // the net is not critical when it has more than two nodes
        // in each partition. Non-critical nets cannot change the
        // gain of a node.

        if (net.m_nodesInPartition.at(fromPartitionIndex) == 1)
        {
            // moving the last node in a partition to 
            // the other side will uncut the net.
            node.m_gain += net.m_weight;
        }

        if (net.m_nodesInPartition.at(toPartitionIndex) == 0)
        {
            // moving a node from an uncut net to
            // another partition will cut the net.
            node.m_gain -= net.m_weight;
        }            
    }
    
}

void FMContainer::calcAndSetNodeGains()
{
    for(auto& node : m_nodes)
    {
        calcAndSetNodeGain(node);
    }
}

void FMContainer::assignNodesToBuckets()
{
    for(auto& node : m_nodes)
    {     
        if (!node.isFixed())
        {
            addNodeToPartitionBucket(node.m_self);
        }
    }
}


int64_t FMPart::distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos)
{
    auto dx = std::max(part.m_region.m_ll.m_x - pos.m_x, int64_t(0));
         dx = std::max(dx, pos.m_x - part.m_region.m_ur.m_x);

    auto dy = std::max(part.m_region.m_ll.m_y - pos.m_y, int64_t(0));
         dy = std::max(dy, pos.m_y - part.m_region.m_ur.m_y);

    return dx+dy;
}


bool FMPart::init(FMContainer &container)
{
    assert(!container.m_nets.empty());
    assert(!container.m_nodes.empty());
    
    // create two partitions each half the size of the region
    // cut in the longest axis
    container.m_partitions[0].m_region = container.m_region;
    container.m_partitions[1].m_region = container.m_region;
    if (container.m_region.width() >= container.m_region.height())
    {
        // cut vertically
        auto cutpos = container.m_region.left() + container.m_region.width() / 2;
        container.m_partitions[0].m_region.setRight(cutpos);    // left half
        container.m_partitions[1].m_region.setLeft(cutpos);     // right half
    }
    else
    {
        //cut horizontally
        auto cutpos = container.m_region.bottom() + container.m_region.height() / 2;
        container.m_partitions[0].m_region.setBottom(cutpos);   // top half
        container.m_partitions[1].m_region.setTop(cutpos);      // bottom half
    }

    for(auto& node : container.m_nodes)
    {
        if (node.m_instance == nullptr)
        {
            doLog(LOG_WARN, "FMPart::init encountered nullptr instance (ID=%d) in node list\n", node.m_self);
            return false;
        }

        // check if the instance has a fixed position.
        // if so, assign the instance/node to the closest
        // partition
        if (node.m_instance->m_placementInfo == ChipDB::PlacementInfo::PLACEMENT_PLACEDANDFIXED)
        {
            auto const* insPtr = node.m_instance;
            auto distanceToPartition0 = distanceToPartition(container.m_partitions[0], insPtr->m_pos);
            auto distanceToPartition1 = distanceToPartition(container.m_partitions[1], insPtr->m_pos);
            if (distanceToPartition0 < distanceToPartition1)
            {
                doLog(LOG_VERBOSE, "  Add pin %s (%lld, %lld) to partition %d\n", insPtr->m_name.c_str(), insPtr->m_pos.m_x, insPtr->m_pos.m_y, 0);
                node.m_partitionId = 0;
            }
            else
            {
                doLog(LOG_VERBOSE, "  Add pin %s (%lld, %lld) to partition %d\n", insPtr->m_name.c_str(), insPtr->m_pos.m_x, insPtr->m_pos.m_y, 1);
                node.m_partitionId = 1;
            }
            node.fix();
            node.lock();
        }
        else
        {
            // randomly assign a partition
            if (std::rand() > (RAND_MAX/2))
            {
                node.m_partitionId = 1;
            }
            else
            {
                node.m_partitionId = 0;
            }
        }
    }

    // set the connected nets of each node
    // and add the node to each net
    for(auto& net : container.m_nets)
    {
        net.m_nodesInPartition[0] = 0;
        net.m_nodesInPartition[1] = 0;

        for(auto nodeId : net.m_nodes)
        {
            auto &node = container.m_nodes.at(nodeId);
            net.m_nodesInPartition.at(node.m_partitionId)++;
        }
    }

    // calculate gains for each node
    // and add to a partition bucket if movable
    container.calcAndSetNodeGains();
    container.assignNodesToBuckets();
    
    return true;
}


int64_t FMPart::cycle(FMContainer &container)
{
    struct FreeListType
    {
        NodeId      m_nodeId;
        GainType    m_totalGain;
    };

    std::vector<FreeListType> freeNodes;

    freeNodes.resize(container.m_nodes.size());

    size_t      freeNodeIdx = 0;
    GainType    totalGain = 0;
    bool updated = false;
    do
    {
        updated = false;
        // for now, we try to balance each partition by selecting the largest
        // partition
        auto partitionId = (container.m_partitions[1].getTotalWeight() > container.m_partitions[0].getTotalWeight()) ? 1 : 0;

        // find the node with the most gain
        auto iter = container.begin(partitionId);
        if (iter != container.end(partitionId))
        {
            updated = true;
            auto nodeId = iter->m_self;
            totalGain += iter->m_gain;

            assert(!iter->isLocked());
            assert(!iter->isFixed());

            freeNodes.at(freeNodeIdx).m_nodeId    = nodeId;  // save the node ID in the free list
            freeNodes.at(freeNodeIdx).m_totalGain = totalGain;
            freeNodeIdx++;

            container.removeNodeFromPartitionBucket(nodeId);
            moveNodeAndUpdateNeighbours(nodeId, container);
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

    // undo the movement of the nodes
    // that come after the maximum gain node in the free list
    for(ssize_t idx=maxTotalGainIdx+1; idx < freeNodeIdx; idx++)
    {
        auto& item = freeNodes.at(idx);            
        auto& node = container.m_nodes.at(item.m_nodeId);

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
            auto& net = container.m_nets.at(netId);
            net.m_nodesInPartition.at(fromPartition)--;
            net.m_nodesInPartition.at(toPartition)++;
        }
    }
    
    // add all the nodes in the free list back into their
    // partition buckets
    for(ssize_t idx=0; idx < freeNodeIdx; idx++)
    {
        auto& item = freeNodes.at(idx);
        auto& node = container.m_nodes.at(item.m_nodeId);
        
        //assert(container.m_partitions[node.m_partitionId].hasNodeInBucket(node.m_self) == false);

        container.calcAndSetNodeGain(node);
        container.addNodeToPartitionBucket(item.m_nodeId);

        // unlock the node, if it is not a fixed node
        if (node.isFixed())
        {
            node.lock();
        }
        else
        {
            node.unlock();
        }
    }   

    // ****************************************************
    // ** calculate the cost of the partitioning         **
    // ****************************************************    

    auto cost = calculateNetCutCost(container);

    return cost;
}

void FMPart::moveNodeAndUpdateNeighbours(NodeId nodeId, FMContainer &container)
{
    auto& node = container.m_nodes.at(nodeId);

    // ************************************
    // ** Move the node
    // ************************************

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

    node.lock();
    node.m_partitionId = toPartitionIndex;    // move node
    
    // ************************************
    // ** Update the neighbour gains
    // ************************************

    for(auto netId : node.m_nets)
    {
        auto& net = container.m_nets.at(netId);

        // check critical net before the move
        if (net.m_nodesInPartition.at(toPartitionIndex) == 0)
        {
            // increment the gains on all the free cells on the net
            for(auto netNodeId : net.m_nodes)
            {
                auto& netNode = container.m_nodes.at(netNodeId);
                if (!netNode.isLocked())
                {                                        
                    container.removeNodeFromPartitionBucket(netNodeId);
                    netNode.m_gain += net.m_weight;
                    container.addNodeToPartitionBucket(netNodeId);
                }
            }
        }
        else if (net.m_nodesInPartition.at(toPartitionIndex) == 1)
        {
            // decrement the gain of the only T cell on the net
            // if it's free
            size_t count = 0;
            for(auto netNodeId : net.m_nodes)
            {
                auto& netNode = container.m_nodes.at(netNodeId);
                if ((netNode.m_partitionId == toPartitionIndex) &&
                    !netNode.isLocked())
                {
                    container.removeNodeFromPartitionBucket(netNodeId);
                    netNode.m_gain -= net.m_weight;
                    container.addNodeToPartitionBucket(netNodeId);
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
        net.m_nodesInPartition.at(toPartitionIndex)++;
        net.m_nodesInPartition.at(fromPartitionIndex)--;

        assert(net.m_nodesInPartition.at(toPartitionIndex) >= 0);
        assert(net.m_nodesInPartition.at(fromPartitionIndex) >= 0);

        // check critical net after the move
        if (net.m_nodesInPartition.at(fromPartitionIndex) == 0)
        {
            // decrement gains of all free cells on the net
            for(auto netNodeId : net.m_nodes)
            {
                auto& netNode = container.m_nodes.at(netNodeId);
                if (!netNode.isLocked())
                {                    
                    container.removeNodeFromPartitionBucket(netNodeId);
                    netNode.m_gain -= net.m_weight;
                    container.addNodeToPartitionBucket(netNodeId);               
                }
            }
        }
        else if (net.m_nodesInPartition.at(fromPartitionIndex) == 1)
        {
            // increment gain of the only F cell on the net
            // if it's free
            size_t count = 0;
            for(auto netNodeId : net.m_nodes)
            {                
                auto& netNode = container.m_nodes.at(netNodeId);
                if ((netNode.m_partitionId == fromPartitionIndex) &&
                    !netNode.isLocked())
                {                    
                    container.removeNodeFromPartitionBucket(netNodeId);
                    netNode.m_gain += net.m_weight;
                    container.addNodeToPartitionBucket(netNodeId);               
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

int64_t FMPart::calculateNetCutCost(const FMContainer &container)
{
    // determine how many times a net is cut
    int64_t cost = 0;
    for(auto const& net : container.m_nets)
    {
        cost += net.m_weight * std::min(net.m_nodesInPartition[0],net.m_nodesInPartition[1]);
    }
    return cost;
}

void FMPart::exportToDot(std::ostream &dotFile, FMContainer &container)
{
    dotFile << "graph G {\n";
    dotFile << "  rankdir=LR;\n"; 
    //os << "  splines=ortho;\n";
    //os << "  node [shape=record style=filled];\n";
    dotFile << "  labelloc=\"t\";\n";
    dotFile << "  fontsize  = 30;\n";

    // write out nodes
    const char *color1 = "red";
    const char *color2 = "green";
    for(auto const& node : container.m_nodes)
    {
        std::stringstream nodeLabel;
        if (node.m_instance->m_insType == ChipDB::InstanceBase::INS_PIN)
        {
            const auto *pinInfo = node.m_instance->getPinInfo(0);
            if (pinInfo == nullptr)
            {
                doLog(LOG_ERROR, "FMPart:: cannot find pinInfo\n");
                continue;
            }

            if (pinInfo->isOutput() || pinInfo->isInput())
            {
                nodeLabel << node.m_instance->m_name;
            }
        }
        else
        {
            nodeLabel << node.m_self;
        }

        dotFile << "  N" << node.m_self << R"([shape="circle", label = ")"  << nodeLabel.str();
        dotFile << "\", color = ";
        if (node.m_partitionId == 0)
        {
            dotFile << color1;
        }
        else
        {
            dotFile << color2;
        }

        if (node.isFixed())
        {
            dotFile << ", style=filled, fillcolor=lightgrey";
        }

        dotFile << " ];\n";
    }

    // write out connections
    for(auto const& net : container.m_nets)
    {
        auto iter = net.m_nodes.begin();
        if (iter == net.m_nodes.end())
        {
            continue;
        }

        auto const& firstNode = container.m_nodes.at(*iter);
        iter++;
        while(iter != net.m_nodes.end())
        {
            auto const& otherNode = container.m_nodes.at(*iter);
            dotFile << "  N" << firstNode.m_self << " -- N" << otherNode.m_self << ";\n";
            iter++;
        }
    }

    dotFile << "\n}\n";
}

#if 0
bool FMPart::doPartitioning(FMContainer &container)
{
    FMContainer ncp0;
    FMContainer ncp1;

    std::unordered_map<NodeId, NodeId> ncp0_node_renumber;  // old node is key
    std::unordered_map<NodeId, NodeId> ncp1_node_renumber;
    std::unordered_map<NetId, NetId>   ncp0_net_renumber;   // old net is key
    std::unordered_map<NetId, NetId>   ncp1_net_renumber;

    //FIXME: mer

    for(auto& node : container.m_nodes)
    {
        // check if node is inside the region
        // or if it is an external node
        //
        //

        if (node.m_partitionId == 0)
        {
            size_t nodeId = ncp0.m_nodes.size();
            ncp0.m_nodes.push_back(node);
            ncp0.m_nodes.back().reset(nodeId);
            ncp0_node_renumber[node.m_self] = nodeId;
        }
        else
        {
            size_t nodeId = ncp0.m_nodes.size();
            ncp1.m_nodes.push_back(node);
            ncp1.m_nodes.back().reset(nodeId);
            ncp1_node_renumber[node.m_self] = nodeId;
        }
    }


    for(auto& net : container.m_nets)
    {

    }
}
#endif

bool FMPart::doPartitioning(ChipDB::Netlist *nl, FMContainer &container)
{
    container.m_nodes.resize(nl->m_instances.size());
    container.m_nets.resize(nl->m_instances.size());

    size_t index = 0;
    for(auto *ins : nl->m_instances)
    {
        if (ins != nullptr)
        {
            container.m_nodes[index].m_weight = ins->instanceSize().m_x;
            ins->m_id = index;
        }
        container.m_nodes[index].m_instance = ins;
        container.m_nodes[index].m_self = index;
        index++;
    }

    index = 0;
    for(auto *net : nl->m_nets)
    {
        net->m_id = index;
        index++;
    }

    for(auto *ins : nl->m_instances)
    {
        for(ssize_t pinIndex=0; pinIndex < ins->getNumberOfPins(); ++pinIndex)
        {
            auto *net = ins->getConnectedNet(pinIndex);
            if (net != nullptr)
            {
                const auto netIndex  = net->m_id;
                const auto nodeIndex = ins->m_id;

                auto& net  = container.m_nets.at(netIndex);
                auto& node = container.m_nodes.at(nodeIndex);

                // Note: this automatically removes duplicate nets
                node.m_nets.push_back(netIndex);

                // Note: this automatically removes duplicate nodes
                net.m_nodes.push_back(nodeIndex);

                net.m_nodesInPartition.at(node.m_partitionId)++;
            }
        }
    }

    // check if there are external pins on the net
    // if so, increase the net weight
    for(auto& net : container.m_nets)
    {
        for(auto nodeId : net.m_nodes)
        {
            auto& node = container.m_nodes.at(nodeId);

            // instance can be nullptr for certain special cells
            // such as __NETCON
            if (node.m_instance == nullptr)
            {
                continue;
            }

            if (node.m_instance->m_insType == ChipDB::InstanceBase::INS_PIN)
            {
                net.m_weight += 4;
                break;
            }
        }
    }

    if (!init(container))
    {
        doLog(LOG_ERROR,"FMPart::init failed\n");
        return false;
    }

    doLog(LOG_VERBOSE, "  Pre-partitioning cost: %lld\n", calculateNetCutCost(container));

    int64_t minCost = std::numeric_limits<int64_t>::max();
    size_t  cyclesSinceMinCostSeen = 0;
    int32_t cycleCount = 0;
    while(cyclesSinceMinCostSeen < 3)
    {        
        auto cost = cycle(container);
        cycleCount++;

        doLog(LOG_VERBOSE,"  Cost of cycle %d = %lld\n", cycleCount, cost);
        if (cost < minCost)
        {
            cyclesSinceMinCostSeen = 0;
            minCost = cost;
            //FIXME: save best partitioning
        }
        cyclesSinceMinCostSeen++;
    }
    doLog(LOG_VERBOSE, "  Post-partitioning cost: %lld\n",minCost);

    return true;
}
