#pragma once

#include <stdint.h>
#include <map>
#include <list>
#include <array>
#include <assert.h>
#include <iostream>

#include "common/dbtypes.h"
#include "netlist/netlist.h"
#include "fmtypes.h"
namespace LunaCore::Partitioner
{

/*

    Recursive partitioning

    for a partition do:
    * build node and net vectors by index / filter on physical node location.
    * create and fill two partitions.
    * run FM algorithm
    * write back the physical positions in the instances (delayed in multi-threaded mode)
    * call partitioner on the two new partitions if the partition size is larger than
      a certain threshold. (or contains more than x number of cells)

*/

using BucketType = typename std::map<GainType, NodeId>;

struct Partition
{
    Partition() : m_totalWeight(0) {}

    ChipDB::Rect64 m_region;

    // gain based buckets, each containing a doubly linked list
    BucketType m_buckets;

    auto getTotalWeight() const
    {
        return m_totalWeight;
    }

    void addToTotalWeight(int64_t weight)
    {
        m_totalWeight += weight;
    }

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

#if 0
    auto begin()
    {
        return m_buckets.rbegin();
    }

    auto end()
    {
        return m_buckets.rend();
    }
#endif

#if 0
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = NodeId;
        using difference_type = std::ptrdiff_t;

        Iterator(Partition &partition, bool atEnd)
            : m_partition(partition)
        {
            m_curNodeId = -1;
            if (!atEnd)
            {
                m_bucketIter = m_partition.m_buckets.rbegin();
                if (m_bucketIter != m_partition.m_buckets.rend())
                {
                    m_curNodeId = m_bucketIter->second;
                }
            }
        }

        bool operator==(const Iterator &other) const
        {
            return m_curNodeId == other.m_curNodeId;
        }

        bool operator!=(const Iterator &other) const
        {
            return m_curNodeId != other.m_curNodeId;
        }

        value_type operator*()
        {
            return m_curNodeId;
        }

#if 0
        value_type operator->()
        {
            return m_curNode;
        }
#endif

        auto getBucketGain() const
        {
            return m_bucketIter->first;
        }

        Iterator& operator++()
        {
            if (m_curNodeId != -1)
            {
                auto nextNodeId = m_curNode->m_next;
                if (nextNodeId == -1)
                {
                    // end of current bucket..
                    m_bucketIter++;
                    if (m_bucketIter == m_partition.m_buckets.rend())
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
        Partition                       &m_partition;
        BucketType::reverse_iterator    m_bucketIter;

        NodeId                          m_curNodeId;
    };
#endif

protected:
    uint64_t           m_totalWeight;
};

class PartitionBucketIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type   = Node;
    using pointer_type = Node*;
    using difference_type = std::ptrdiff_t;

    PartitionBucketIterator(std::vector<Node> &nodes, Partition &partition, bool atEnd) : m_nodes(nodes), m_partition(partition)
    {
        m_curNode = nullptr;
        if (!atEnd)
        {
            m_bucketIter = partition.m_buckets.rbegin();
            if (m_bucketIter != partition.m_buckets.rend())
            {
                m_curNode = &nodes.at(m_bucketIter->second);
            }
        }
        else
        {
            m_bucketIter = partition.m_buckets.rend();
        }
    }

    PartitionBucketIterator& operator++()
    {
        if (m_curNode != nullptr)
        {
            auto nextNodeId = m_curNode->m_next;
            if (nextNodeId == -1)
            {
                // end of current bucket..
                m_bucketIter++;
                if (m_bucketIter == m_partition.m_buckets.rend())
                {
                    m_curNode = nullptr;
                    return (*this); // early exit for end situation
                }
                nextNodeId = m_bucketIter->second;
            }

            m_curNode = &m_nodes.at(nextNodeId);
        }

        return (*this);
    }


    bool operator==(const PartitionBucketIterator &other) const
    {
        return m_curNode == other.m_curNode;
    }

    bool operator!=(const PartitionBucketIterator &other) const
    {
        return m_curNode != other.m_curNode;
    }

    pointer_type operator*()
    {
        return m_curNode;
    }

    pointer_type operator->()
    {
        return m_curNode;
    }

protected:
    BucketType::reverse_iterator m_bucketIter;
    Partition                   &m_partition;
    std::vector<Node>           &m_nodes;
    Node                        *m_curNode;

};

class FMContainer
{
public:
    ChipDB::Rect64          m_region;
    std::array<Partition,2> m_partitions;
    std::vector<Node>       m_nodes;        ///< all nodes required for the region
    std::vector<Net>        m_nets;         ///< all nets required for the region

    void addNodeToPartitionBucket(NodeId nodeId)
    {        
        auto& node = m_nodes.at(nodeId);
        auto& partition = m_partitions.at(node.m_partitionId);

        // check if bucket already exists
        if (partition.hasBucket(node.m_gain))
        {
            auto oldNodeId = partition.m_buckets[node.m_gain];
            auto& oldNode = m_nodes.at(oldNodeId);
            oldNode.m_prev = node.m_self;
            node.m_next = oldNodeId;
            node.m_prev = -1;
        }

        partition.m_buckets[node.m_gain] = node.m_self;
        partition.addToTotalWeight(node.m_weight);
    }

    void removeNodeFromPartitionBucket(NodeId nodeId)
    {        
        auto& node = m_nodes.at(nodeId);
        auto& partition = m_partitions.at(node.m_partitionId);

        if (node.m_next != -1)
        {
            // unlink the next node
            auto& nextNode = m_nodes.at(node.m_next);
            nextNode.m_prev = node.m_prev;
        }

        if (node.m_prev != -1)
        {
            // unlink the previous node
            auto &prevNode = m_nodes.at(node.m_prev);
            prevNode.m_next = node.m_next;
        }      

        // check if the node was the head node
        // and unlink from the bucket side
        auto iter = partition.m_buckets.find(node.m_gain);
        if (iter->second == node.m_self)
        {
            iter->second = node.m_next;

            // if there is no next node
            // destroy the bucket!
            if (node.m_next == -1)
            {
                partition.removeBucket(node.m_gain);
            }
        }    

        node.resetLinks();  
        partition.addToTotalWeight(-node.m_weight);
    }

    auto begin(PartitionId partitionId)
    {
        assert((partitionId < 2) && (partitionId >= 0));
        return PartitionBucketIterator(m_nodes, m_partitions[partitionId], false);
    }

    auto end(PartitionId partitionId)
    {
        assert((partitionId < 2) && (partitionId >= 0));
        return PartitionBucketIterator(m_nodes, m_partitions[partitionId], true);
    }

    void calcAndSetNodeGain(Node &node);
    void calcAndSetNodeGains();
    void assignNodesToBuckets();
};

class FMPart
{
public:
    FMPart()
    {
    }

    /** export the current partitioning to a Graphviz dot file for viewing */
    static void exportToDot(std::ostream &dotFile, FMContainer &container);

    /** perform partitioning of a netlist.
     *  This assumes the partition regions have been set.
     * 
     *  Instances that have placement status of PLACEMENT_PLACED_AND_FIXED
     *  will be assigned to the closest partition.
     * 
     *  Other instances will be able to move between partitions.
    */
    //

    bool doPartitioning(ChipDB::Netlist *nl, FMContainer &container);
    bool doPartitioning(FMContainer &container);

protected:
    /** returns the total number of cuts in each net */
    int64_t calculateNetCutCost(const FMContainer &container) const;

    /** fill the two partitions at random with nodes from 
     *  the netlist */
    bool init(FMContainer &container);

    /** perform one FM partitioning cycle
     *  assumes node, nets and partitions have been generated
     *  by init(..)
     * 
     *  returns the cost of the partitioning
     */
    int64_t cycle(FMContainer &container);

    /** get minimum manhattan distance between a rectangular partition
     *  and the position specified
    */
    int64_t distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos);
    
    void moveNodeAndUpdateNeighbours(NodeId nodeId, FMContainer &container);
};

};