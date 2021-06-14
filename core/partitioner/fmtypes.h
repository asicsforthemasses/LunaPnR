#pragma once

#include <stdint.h>
#include <vector>

namespace LunaCore::Partitioner
{

    using PartitionId   = int32_t;
    using NetId         = int32_t;
    using NodeId        = int32_t;
    using GainType      = int32_t;

    struct Node
    {
        std::vector<NetId>  m_nets;             ///< nets connected to this nodes
        PartitionId         m_partitionId;      ///< current location of the node: partition 0 or 1
        int64_t             m_weight;           ///< weight of the node (probably cell width instead of area)
        int64_t             m_gain;             ///< change in the number of net cuts when node is moved to the other partition
        bool                m_locked;           ///< if true, the node is locked/unmovable during the FM partitioning cycle
        bool                m_fixed;            ///< if true, the node can never be moved

        // IDs for gain based bucket list implementation
        NodeId              m_next;
        NodeId              m_prev;
        NodeId              m_self;

        constexpr bool isLinked() const noexcept
        {
            return (m_next != -1) || (m_prev != -1);
        }

        void reset(NodeId self)
        {
            resetLinks();
            m_gain = 0;
            m_self = self;
            m_locked = false;
            m_fixed  = false;
            m_partitionId = -1;
        }

        void setFixed()
        {
            m_locked = true;
            m_fixed  = true;
        }

        void resetLinks()
        {
            m_next = -1;
            m_prev = -1;
        }
    };

    struct Net
    {
        std::vector<NodeId> m_nodes;
        int32_t             m_weight;
        int32_t             m_nodesInPartition[2];
    };

};