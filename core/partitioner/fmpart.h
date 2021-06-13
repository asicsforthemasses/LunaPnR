#pragma once

#include <stdint.h>
#include <map>
#include <list>
#include <array>
#include "common/dbtypes.h"
#include "netlist/netlist.h"
#include "fmtypes.h"
namespace LunaCore::Partitioner
{

class FMPart
{
public:
    FMPart()
    {
        m_partitions.emplace_back(Partition(m_nodes));
        m_partitions.emplace_back(Partition(m_nodes));
    }

    using BucketType = typename std::map<GainType, NodeId>;

    struct Partition
    {
        Partition(std::vector<Node> &nodes) : m_nodes(nodes) {}

        ChipDB::Rect64 m_region;

        // gain based buckets, each containing a doubly linked list
        BucketType m_buckets;

        /** check if a bucket for a specific gain exists */
        bool hasBucket(const GainType gain) const
        {
            return (m_buckets.find(gain) != m_buckets.end());
        }

        /** remove a bucket for a specific gain 
         *  note: does not unlink the nodes contained within!
        */
        void removeBucket(const GainType gain)
        {
            if (hasBucket(gain))
            {
                m_buckets.erase(m_buckets.find(gain));
            }
        }
        class Iterator
        {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = Node*;
            using difference_type = std::ptrdiff_t;
            //using pointer = Node*;
            //using reference = blDataType&;

            Iterator(Partition &partition, bool atEnd) 
                : m_partition(partition)
            {
                m_curNode = nullptr;
                if (!atEnd)
                {
                    m_bucketIter = m_partition.m_buckets.begin();
                    if (m_bucketIter != m_partition.m_buckets.end())
                    {
                        m_curNode = &m_partition.m_nodes.at(m_bucketIter->second);
                    }
                }
            }

            bool operator==(const Iterator &other) const
            {
                return m_curNode == other.m_curNode;
            }

            bool operator!=(const Iterator &other) const
            {
                return m_curNode != other.m_curNode;
            }

            Node* operator*()
            {
                return m_curNode;
            }

            Node* operator->()
            {
                return m_curNode;
            }

            Iterator& operator++()
            {
                if (m_curNode != nullptr)
                {
                    auto nextNodeId = m_curNode->m_next;
                    if (nextNodeId == -1)
                    {
                        // end of current bucket..
                        m_bucketIter++;
                        if (m_bucketIter == m_partition.m_buckets.end())
                        {
                            m_curNode = nullptr;
                            return (*this); // early exit for end situation
                        }
                        nextNodeId = m_bucketIter->second;
                    }

                    m_curNode = &m_partition.m_nodes.at(nextNodeId);
                }

                return (*this);
            }

        protected:
            Partition               &m_partition;
            BucketType::iterator    m_bucketIter;
            Node                    *m_curNode;
        };

    auto begin() 
    {
        return Iterator(*this, false);
    }

    auto end() 
    {
        return Iterator(*this, true);
    }

    protected:
        std::vector<Node> &m_nodes;
    };

    bool init(ChipDB::Netlist *nl);

    std::vector<Node>       m_nodes;    ///< storage for all nodes in the netlist
    std::vector<Net>        m_nets;     ///< storage for all nets in the netlist

    std::vector<Partition> m_partitions;

protected:
    int64_t distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos);
    
    bool addNodeToPartitionBucket(const NodeId nodeId);
    bool removeNodeFromPartitionBucket(const NodeId nodeId);

};

};