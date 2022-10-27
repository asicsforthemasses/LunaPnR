#pragma once
#include <string>
#include <array>
#include <list>
#include <memory>

#include "common/dbtypes.h"
#include "netlist/net.h"
#include "design/design.h"

namespace LunaCore::CTS
{

struct CTSNode
{
    ChipDB::ObjectKey m_insKey; ///< instance key into netlist
};

class CTSNodeList
{
public:

    using ContainerType = std::vector<CTSNode>;

    CTSNodeList() = default;

    enum class Axis
    {
        X,
        Y
    };

    /** split the node list into two equal parts, based on the median location along the specified axis */
    [[nodiscard]] std::pair<CTSNodeList, CTSNodeList> split(const ChipDB::Netlist &netlist, Axis axis);

    /** return the center of gravity / mean of the nodes */
    [[nodiscard]] ChipDB::Coord64 mean(const ChipDB::Netlist &netlist) const;

    [[nodiscard]] std::size_t size() const noexcept
    {
        return m_nodes.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_nodes.empty();
    }

    void reserve(std::size_t N)
    {
        m_nodes.reserve(N);
    }

    void clear() noexcept
    {
        m_nodes.clear();
    }

    const CTSNode& front() const noexcept
    {
        return m_nodes.front();
    }

    void push_back(const ChipDB::InstanceObjectKey key)
    {
        m_nodes.push_back({key});
    }

    void insertAtEnd(ContainerType::const_iterator s, ContainerType::const_iterator e);

protected:
    /** sort the nodes along the specified axis */
    void sortAlongAxis(const ChipDB::Netlist &netlist, const Axis axis);

    ContainerType m_nodes;
};

/**
 *  CTS based on https://dl.acm.org/doi/pdf/10.1145/123186.123406
*/
class MeanAndMedianCTS
{
public:
    using SegmentIndex = int;

    struct Segment
    {
        ChipDB::Coord64 m_start;
        ChipDB::Coord64 m_end;
        SegmentIndex m_parent{-1};
        ChipDB::InstanceObjectKey m_insKey{ChipDB::ObjectNotFound};
    };

    class SegmentList
    {
    public:
        SegmentList() = default;

        SegmentList(const SegmentList &) = delete;
        SegmentList operator=(const SegmentList &) = delete;

        SegmentList(SegmentList &&) = default;

        SegmentIndex createSegment(const ChipDB::Coord64 &s, const ChipDB::Coord64 &e,
            SegmentIndex parentIndex)
        {
            return createSegment(s,e,parentIndex, ChipDB::ObjectNotFound);
        }

        SegmentIndex createSegment(const ChipDB::Coord64 &s, const ChipDB::Coord64 &e,
            SegmentIndex parentIndex, ChipDB::InstanceObjectKey insKey)
        {
            m_segments.push_back({s,e, parentIndex, insKey});
            return m_segments.size() - 1;
        }

        [[nodiscard]] bool empty() const noexcept { return m_segments.empty(); }
        [[nodiscard]] std::size_t size() const noexcept { return m_segments.size(); }

        const Segment& at(std::size_t index) const {return m_segments.at(index); }
        Segment& at(std::size_t index) {return m_segments.at(index); }

        auto begin() const noexcept
        {
            return m_segments.begin();
        }

        auto end() const noexcept
        {
            return m_segments.end();
        }

    protected:
        std::vector<Segment> m_segments;
    };

    std::optional<SegmentList> generateTree(const std::string &clockNetName, ChipDB::Netlist &netlist);

protected:
    void recursiveSubdivision(const ChipDB::Netlist &netlist, CTSNodeList &nodes, 
        SegmentList &segments, SegmentIndex topSegIndex = 0);
};

};