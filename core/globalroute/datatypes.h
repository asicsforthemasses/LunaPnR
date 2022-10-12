// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
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

    struct NetSegment
    {
        GCellCoord      m_start{0,0};   ///< grid starting coordinate
        GCellCoordType  m_length{1};    ///< length in blocks
        Direction       m_dir{Direction::Undefined};
        NetSegment*     m_parent{nullptr};
    };

    struct SegmentList
    {
        NetSegment* createNewSegment(const GCellCoord &start, Direction dir, NetSegment *parent = nullptr)
        {
            auto segPtr = &m_segments.emplace_back();
            segPtr->m_start = start;
            segPtr->m_dir = dir;
            segPtr->m_length = 1;
            segPtr->m_parent = parent;
            return segPtr;
        }
        
        std::list<NetSegment> m_segments;
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
