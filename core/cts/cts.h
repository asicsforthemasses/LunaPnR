#pragma once
#include <string>
#include <array>
#include <list>
#include <memory>

#include "database/database.h"

namespace LunaCore::CTS
{

struct CTSNull
{
};

/** a terminal node on the clock network */
struct CTSNode
{
    ChipDB::ObjectKey m_insKey{ChipDB::ObjectNotFound}; ///< instance key into netlist
    ChipDB::ObjectKey m_pinKey{ChipDB::ObjectNotFound}; ///< instance pin key connected to clock
    float             m_capacitance{0.0f};

    [[nodiscard]] constexpr bool isValid() const noexcept
    {
        return (m_insKey != ChipDB::ObjectNotFound) && (m_pinKey != ChipDB::ObjectNotFound);
    }
};


/** list of terminal nodes on the clock network */
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

    void push_back(const ChipDB::InstanceObjectKey insKey,
        const ChipDB::InstanceObjectKey pinKey,
        float pinCapacitance)
    {
        m_nodes.push_back(CTSNode{insKey, pinKey, pinCapacitance});
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

    /** routing segment on the clock network
     *  if m_insKey != ChipDB::ObjectNotFound, it means
     *  that there is a buffer or terminal cell at
     *  the end.
    */
    struct Segment
    {
        ChipDB::Coord64 m_start;
        ChipDB::Coord64 m_end;
        SegmentIndex    m_parent{-1};
        CTSNode         m_cell;         ///< clock end point or buffer cell
        int             m_level{0};     ///< depth level
        std::vector<SegmentIndex> m_children;

        void addChild(ChipDB::InstanceObjectKey key)
        {
            m_children.push_back(key);
        }

        /** true if the clock network ends at this segment */
        [[nodiscard]] bool isTerminal() const noexcept
        {
            return m_children.empty();
        }

        /** true if the clock network has a cell (buffer or clocked cell) at this segment end */
        [[nodiscard]] constexpr bool hasCell() const noexcept
        {
            return m_cell.isValid();
        }

        /** check if the segment is valid.
         *  segments without a cell are always valid
         *  terminal segments _must_ have a valid cell
        */
        [[nodiscard]] bool isValid() const noexcept
        {
            if (!isTerminal()) return true;
            return (isTerminal() && hasCell());
        }
    };

    /** a list of segments, describing the clock network. */
    class SegmentList
    {
    public:
        SegmentList() = default;

        SegmentList(const SegmentList &) = delete;
        SegmentList operator=(const SegmentList &) = delete;

        SegmentList(SegmentList &&) = default;

        /** create a segment without a cell attached */
        SegmentIndex createSegment(const ChipDB::Coord64 &s, const ChipDB::Coord64 &e,
            SegmentIndex parentIndex, int level)
        {
            auto &seg = m_segments.emplace_back();
            seg.m_start = s;
            seg.m_end   = e;
            seg.m_parent = parentIndex;
            seg.m_level = level;
            return m_segments.size() - 1;
        }

        /** create a segment with a cell attached */
        SegmentIndex createSegment(const ChipDB::Coord64 &s, const ChipDB::Coord64 &e,
            SegmentIndex parentIndex, const CTSNode &ctsnode, int level)
        {
            auto &seg = m_segments.emplace_back();
            seg.m_start = s;
            seg.m_end   = e;
            seg.m_cell  = ctsnode;
            seg.m_parent = parentIndex;
            seg.m_level = level;
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

    /* Returns a list of routing segments describing the clock tree.
       All instances must be valid. All pins must be valid.
    */
    std::optional<SegmentList> generateTree(const std::string &clockNetName, ChipDB::Netlist &netlist);

    float calcSegmentLoadCapacitance(SegmentIndex index, const SegmentList &segments, const ChipDB::Netlist &netlist) const;

    struct CTSInfo
    {
        ChipDB::NetObjectKey m_clkNetKey{ChipDB::ObjectNotFound};
        std::shared_ptr<ChipDB::Cell> m_bufferCell;
        ChipDB::PinObjectKey m_inputPinKey{ChipDB::ObjectNotFound};
        ChipDB::PinObjectKey m_outputPinKey{ChipDB::ObjectNotFound};
        float       m_pinCapacitance{0.0f};
        float       m_maxCap{0.0f};
    };

    using SinkList = std::list<ChipDB::Net::NetConnect>;

    struct BufferResult
    {
        SinkList m_list;
        float    m_totalCapacitance{0.0};
    };

    BufferResult insertBuffers(
        SegmentList &segments,
        SegmentIndex segIndex,
        ChipDB::Netlist &netlist,
        const CTSInfo &ctsInfo);

protected:

    void recursiveSubdivision(const ChipDB::Netlist &netlist, CTSNodeList &nodes,
        SegmentList &segments, SegmentIndex topSegIndex = 0, int level = 0);
};

};
