#pragma once

#include <stdint.h>
#include <map>
#include <list>
#include <array>
#include "common/dbtypes.h"
#include "netlist/netlist.h"

namespace LunaCore
{

class FMPart
{
public:

    struct Net;         // predeclaration

    using NetId     = int32_t;
    using NodeId    = int32_t;
    using GainType  = int32_t;

    using BucketType = typename std::map<GainType, NodeId>;

    struct Partition
    {
        ChipDB::Rect64 m_region;

        // gain based buckets, each containing a doubly linked list
        BucketType m_buckets;
    };

    struct Node
    {
        std::vector<NetId>  m_nets;             ///< nets connected to this nodes
        uint32_t            m_partitionId;      ///< current location of the node: partition 0 or 1
        int64_t             m_weight;           ///< weight of the node (probably cell width instead of area)
        int64_t             m_gain;             ///< change in the number of net cuts when node is moved to the other partition
        bool                m_locked;           ///< if true, the node is unmovable

        // IDs for gain based bucket list implementation
        NodeId              m_next;
        NodeId              m_prev;
        NodeId              m_self;

        constexpr bool isLinked() const noexcept
        {
            return (m_next != -1) || (m_prev != -1);
        }
    };

    struct Net
    {
        std::vector<NodeId> m_nodes;
        int32_t             m_weight;
        int32_t             m_nodesInPartition[2];
    };

    bool init(ChipDB::Netlist *nl);

    std::vector<Node>       m_nodes;    ///< storage for all nodes in the netlist
    std::vector<Net>        m_nets;     ///< storage for all nets in the netlist

    std::array<Partition, 2> m_partitions;

protected:
    int64_t distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos);
    
    bool addNodeToPartitionBucket(const NodeId nodeId);
    bool removeNodeFromPartitionBucket(const NodeId nodeId);

};

};