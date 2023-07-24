// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <list>
#include <iostream>
#include "common/dbtypes.h"

namespace LunaCore::GlobalRouter
{
    using GCellCoordType = ChipDB::CoordType;
    using GCellCoord = ChipDB::Coord64;
    using PathCostType = int64_t;

    struct TrackInfo
    {
        int horizontal;
        int vertical;
    };    

    enum class Predecessor : uint16_t
    {
        Undefined = 0,
        North = (1<<8),
        South = (2<<8),
        East  = (3<<8),
        West  = (4<<8)
    };

    constexpr GCellCoord north(const GCellCoord &p) {return {p.m_x, p.m_y+1}; }
    constexpr GCellCoord south(const GCellCoord &p) {return {p.m_x, p.m_y-1}; }
    constexpr GCellCoord east(const GCellCoord &p) {return {p.m_x-1, p.m_y}; }
    constexpr GCellCoord west(const GCellCoord &p) {return {p.m_x+1, p.m_y}; }

    enum class Direction
    {
        Undefined, North, South, East, West
    };

    constexpr Direction predecessorToDirection(const Predecessor &p) noexcept
    {
        switch(p)
        {
        case Predecessor::North:
            return Direction::North;
        case Predecessor::South:
            return Direction::South;
        case Predecessor::East:
            return Direction::East;
        case Predecessor::West:
            return Direction::West;
        default:
            return Direction::Undefined;
        };
    }

    constexpr Predecessor directionToPredecessor(const Direction &d) noexcept
    {
        switch(d)
        {
        case Direction::North:
            return Predecessor::North;
        case Direction::South:
            return Predecessor::South;
        case Direction::East:
            return Predecessor::East;
        case Direction::West:
            return Predecessor::West;
        default:
            return Predecessor::Undefined;
        };
    }

    struct NetSegment
    {
        GCellCoord      m_start{0,0};   ///< grid starting coordinate
        GCellCoordType  m_length{1};    ///< length in blocks
        Direction       m_dir{Direction::Undefined};
        NetSegment*     m_parent{nullptr};

        [[nodiscard]] constexpr GCellCoord endPoint() const noexcept
        {
            switch(m_dir)
            {
            case Direction::East: return {m_start.m_x + m_length, m_start.m_y};
            case Direction::West: return {m_start.m_x - m_length, m_start.m_y};
            case Direction::North: return {m_start.m_x, m_start.m_y + m_length};
            case Direction::South: return {m_start.m_x, m_start.m_y - m_length};
            default:
                return m_start;
            }
        }
    };

    /** Segment list that owns the Segments
     *  
    */
    class SegmentList
    {
    public:
        SegmentList() = default;                // creatable
        SegmentList(SegmentList&&) = default;   // moveable 

        SegmentList(const SegmentList&) = delete;       // non-copyable
        SegmentList& operator=(const SegmentList&) = delete;

        virtual ~SegmentList()
        {
            for(auto ptr : m_segments)
            {
                delete ptr;
            }
        }

        /** create a new segment with length = 1 and return a pointer to it. Ownership remains with this SegmentList. */
        NetSegment* createNewSegment(const GCellCoord &start, Direction dir, NetSegment *parent = nullptr)
        {
            m_segments.emplace_back(new NetSegment());
            m_segments.back()->m_start = start;
            m_segments.back()->m_dir = dir;
            m_segments.back()->m_length = 1;
            m_segments.back()->m_parent = parent;
            return m_segments.back();
        }

        /** append a list of new segments at the end of this SegmentList and clear the given list*/
        void absorb(SegmentList &newSegments)
        {   
            m_segments.insert(m_segments.end(), newSegments.m_segments.begin(), newSegments.m_segments.end());
            newSegments.m_segments.clear();
        }

        [[nodiscard]] auto begin() const noexcept
        {
            return m_segments.begin();
        }

        [[nodiscard]] auto end() const noexcept
        {
            return m_segments.end();
        }

        [[nodiscard]] std::size_t size() const noexcept
        {
            return m_segments.size();
        }

    protected:
            std::vector<NetSegment*> m_segments;
    };
};

inline std::ostream& operator<< (std::ostream &os, const LunaCore::GlobalRouter::Direction &dir)
{
    switch(dir)
    {
    case LunaCore::GlobalRouter::Direction::East:
        os << "East";
        break;
    case LunaCore::GlobalRouter::Direction::West:
        os << "West";
        break;     
    case LunaCore::GlobalRouter::Direction::North:
        os << "North";
        break;
    case LunaCore::GlobalRouter::Direction::South:
        os << "South";
        break;                  
    case LunaCore::GlobalRouter::Direction::Undefined:
        os << "Undefined";
        break;        
    }
    return os;
}
