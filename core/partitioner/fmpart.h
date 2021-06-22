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
        Partition(std::vector<Node> &nodes) : m_nodes(nodes), m_totalWeight(0) {}

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

        /** add a node to the head of the bucket list.
        */
        void addNode(Node &node)
        {
            // check if bucket already exists
            if (hasBucket(node.m_gain))
            {
                auto oldNodeId = m_buckets[node.m_gain];
                auto& oldNode = m_nodes.at(oldNodeId);
                oldNode.m_prev = node.m_self;
                node.m_next = oldNodeId;
                node.m_prev = -1;
            }

            m_buckets[node.m_gain] = node.m_self;
            m_totalWeight += node.m_weight;
        }

        void removeNode(Node &node)
        {
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
            auto iter = m_buckets.find(node.m_gain);
            if (iter->second == node.m_self)
            {
                iter->second = node.m_next;

                // if there is no next node
                // destroy the bucket!
                if (node.m_next == -1)
                {
                    removeBucket(node.m_gain);
                }
            }    

            // sanity check for debugging
            #if 0
            for(auto const& bucket : m_buckets)
            {
                if (bucket.second == node.m_self)
                {
                    assert(false);
                }
            }
            #endif

            node.resetLinks();  
            m_totalWeight -= node.m_weight;
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
                    m_bucketIter = m_partition.m_buckets.rbegin();
                    if (m_bucketIter != m_partition.m_buckets.rend())
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

            auto getBucketGain() const
            {
                return m_bucketIter->first;
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
            Node                            *m_curNode;
        };

        auto begin() 
        {
            return Iterator(*this, false);
        }

        auto end() 
        {
            return Iterator(*this, true);
        }

        /** check if node is in one of the buckets 
         *  Note: this is a slow function and is only used
         *        for testing.
        */
        bool hasNodeInBucket(NodeId nodeId)
        {
            for(auto nodePtr : *this)
            {
                if (nodePtr->m_self == nodeId)
                {
                    return true;
                }
            }

            return false;
        }

    protected:
        uint64_t           m_totalWeight;
        std::vector<Node> &m_nodes;
    };

    /** export the current partitioning to a Graphviz dot file for viewing */
    void exportToDot(std::ostream &dotFile);

    /** perform partitioning of a netlist.
     *  This assumes the partition regions have been set.
     * 
     *  Instances that have placement status of PLACEMENT_PLACED_AND_FIXED
     *  will be assigned to the closest partition.
     * 
     *  Other instances will be able to move between partitions.
    */
    bool doPartitioning(ChipDB::Netlist *nl);

    std::vector<Node>       m_nodes;        ///< storage for all nodes in the netlist
    std::vector<Net>        m_nets;         ///< storage for all nets in the netlist
    std::vector<Partition>  m_partitions;   ///< holds bucket lists for each partition etc.

protected:
    /** returns the total number of cuts in each net */
    int64_t calculateCost() const;

    /** fill the two partitions at random with nodes from 
     *  the netlist */
    bool init(ChipDB::Netlist *nl);

    /** perform one FM partitioning cycle
     *  assumes node, nets and partitions have been generated
     *  by init(..)
     * 
     *  returns the cost of the partitioning
     */
    int64_t cycle();

    /** add a node to the partition / bucket list.
     *  the data within the node is used to select
     *  the correct partition based on gain */
    void addNodeToBucket(NodeId nodeId)
    {
        //std::cout << "Add node " << nodeId << "\n";
        
        auto& node = m_nodes.at(nodeId);
        m_partitions.at(node.m_partitionId).addNode(node);
    }

    /** remove a node from the partition / bucket list.
     *  the data within the node is used to select
     *  the correct partition based on gain */
    void removeNodeFromBucket(NodeId nodeId)
    {
        //std::cout << "Remove node " << nodeId << "\n";

        auto& node = m_nodes.at(nodeId);
        m_partitions.at(node.m_partitionId).removeNode(node);

        assert(m_partitions.at(node.m_partitionId).hasNodeInBucket(nodeId) == false);
    }

    void calcNodeGain(Node &node);

    /** get minimum manhattan distance between a rectangular partition
     *  and the position specified
    */
    int64_t distanceToPartition(const Partition &part, const ChipDB::Coord64 &pos);
    
    void moveNodeAndUpdateNeighbours(NodeId nodeId);
};

};