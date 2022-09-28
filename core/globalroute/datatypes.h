// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
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

};
