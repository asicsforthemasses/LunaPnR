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
    
    struct Partition
    {
        ChipDB::Rect64 m_region;
    };

    using NetId  = int32_t;
    using NodeId = int32_t;

    struct Node
    {
        std::vector<NetId>  m_nets;         ///< nets connected to this node
        uint32_t            m_partitionId;  ///< 0 or 1 
        int64_t             m_weight;       ///< weight of the node (probably width as cell height is same for all cells)
        bool                m_locked;       ///< if true, the node is unmovable
    };

    struct Net
    {
        std::vector<NodeId> m_nodes;
        int32_t             m_weight;
    };

    bool init(ChipDB::Netlist *nl);
    int64_t distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos);

    std::vector<Node>       m_nodes;
    std::vector<Net>        m_nets;

    Partition m_part1;
    Partition m_part2;
};

};