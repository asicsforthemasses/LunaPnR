#pragma once

#include <stdint.h>
#include <map>
#include <list>
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

    struct Partition
    {
        ChipDB::Rect64 m_region;

        // gain based buckets, each containing a doubly linked list
        std::map<GainType, NodeId> m_buckets;
    };

    struct Node
    {
        std::vector<NetId>  m_nets;             ///< nets connected to this node
        uint32_t            m_partitionId;      ///< current location of the node: partition 1 or 2
        int64_t             m_weight;           ///< weight of the node (probably width as cell height is same for all cells)
        int64_t             m_gain;             ///< change in the number of net cuts when node is moved to the other partition
        bool                m_locked;           ///< if true, the node is unmovable

        // IDs for gain based bucket list implementation
        NodeId              m_next;
        NodeId              m_prev;
        NodeId              m_self;
    };

    struct Net
    {
        std::vector<NodeId> m_nodes;
        int32_t             m_weight;
        int32_t             m_nodesInPartition[2];
    };

    using bucket = std::list<NodeId>;
    std::map<int32_t, bucket> m_gains;

    enum CutState
    {
        NO_CHANGE,
        WILL_BECOME_CUT,
        WILL_BECOME_UNCUT
    };

    bool init(ChipDB::Netlist *nl);
    int64_t distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos);

    std::vector<Node>       m_nodes;    ///< storage for all nodes in the netlist
    std::vector<Net>        m_nets;     ///< storage for all nets in the netlist

    Partition m_part1;
    Partition m_part2;
};

};