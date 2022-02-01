/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#pragma once

#include <stdint.h>
#include <vector>
#include <algorithm>

namespace LunaCore::Partitioner
{

    using PartitionId   = int32_t;
    using NetId         = int32_t;
    using NodeId        = int32_t;
    using GainType      = int32_t;

    /** vector that only stores unique objects */
    template<class T>
    class DedupVector
    {
    public:
        void resize(size_t N)
        {
            m_data.resize(N);
        }

        auto begin() 
        {
            return m_data.begin();
        }

        auto end() 
        {
            return m_data.end();
        }

        auto begin() const
        {
            return m_data.begin();
        }

        auto end() const
        {
            return m_data.end();
        }

        void push_back(const T &item)
        {
            auto iter = std::find_if(m_data.begin(), m_data.end(),
                [item](auto const& obj)
                {
                    return obj == item;
                }
            );
            
            if (iter == m_data.end())
                m_data.push_back(item);
        }

        T& at(size_t index)
        {
            return m_data.at(index);
        }

        const T& at(size_t index) const
        {
            return m_data.at(index);
        }

        size_t size() const
        {
            return m_data.size();
        }

    protected:
        std::vector<T> m_data;
    };

    struct Node
    {
        Node() : m_partitionId(0), m_bestPartitionId(0), m_weight(0), m_gain(0), 
            m_next(-1), m_prev(-1), m_instance(nullptr),
            m_self(-1), m_visited(false), m_flags(0) {}

        DedupVector<NetId>  m_nets;             ///< nets connected to this node
        PartitionId         m_partitionId;      ///< current location of the node: partition 0 or 1
        PartitionId         m_bestPartitionId;  ///< best location of the node: partition 0 or 1
        int64_t             m_weight;           ///< weight of the node (probably cell width instead of area)
        int64_t             m_gain;             ///< change in the number of net cuts when node is moved to the other partition

        std::shared_ptr<ChipDB::InstanceBase> m_instance;

        // IDs for gain based bucket list implementation
        NodeId              m_next;
        NodeId              m_prev;
        NodeId              m_self;

        bool                m_visited;

        /** returns true if the node is part of a bucket list */
        constexpr bool isLinked() const noexcept
        {
            return (m_next != -1) || (m_prev != -1);
        }

        constexpr bool isLocked() const noexcept
        {
            return (m_flags & c_lockedFlag) != 0;
        }

        constexpr bool isFixed() const noexcept
        {
            return (m_flags & c_fixedFlag) != 0;
        }

        void lock()
        {
            m_flags |= c_lockedFlag;
        }

        void fix()
        {
            m_flags |= c_fixedFlag;
        }

        void unlock()
        {
            m_flags &= ~c_lockedFlag;
        }

        void unfix()
        {
            m_flags &= ~c_fixedFlag;
        }

        void reset(NodeId self)
        {
            resetLinks();
            m_gain = 0;
            m_self = self;
            m_flags = 0;
            m_partitionId = -1;
        }

        void resetLinks()
        {
            m_next = -1;
            m_prev = -1;
        }

    protected:
        static const uint8_t c_lockedFlag = 1;
        static const uint8_t c_fixedFlag  = 2;
        uint8_t m_flags;
    };

    struct Net
    {
        Net() : m_visited(false), m_weight(1), m_nodesInPartition{0,0} {}

        DedupVector<NodeId>     m_nodes;
        int32_t                 m_weight;
        std::array<int32_t,2>   m_nodesInPartition;
        bool                    m_visited;
    };

};