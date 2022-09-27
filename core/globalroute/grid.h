// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <vector>
#include "datatypes.h"

namespace LunaCore::GlobalRouter
{

/** global grid cell */
struct GCell
{
    uint16_t        m_capacity{0};
    uint16_t        m_flags{0};
    PathCostType    m_cost{0};

    constexpr bool isSource() const {return (m_flags & sourceFlag) != 0; }
    constexpr bool isTarget() const {return (m_flags & targetFlag) != 0; }
    constexpr bool isReached() const {return (m_flags & reachedFlag) != 0; }
    constexpr bool isValid() const {return (m_flags & invalidFlag) != 0; }

    constexpr void resetFlags()     { m_flags = 0; }
    constexpr void setReached()     { m_flags |= reachedFlag; }
    constexpr void clearReached()   { m_flags &= ~reachedFlag; }
    //constexpr void setExpanded()    { m_flags |= expandedFlag; }
    //constexpr void clearExpanded()  { m_flags &= ~expandedFlag; }
    constexpr void setTarget()      { m_flags |= targetFlag; }
    constexpr void clearTarget()    { m_flags &= ~targetFlag; }
    constexpr void setSource()      { m_flags |= sourceFlag; }
    constexpr void clearSource()    { m_flags &= ~sourceFlag; }
    //constexpr void setMark()        { m_flags |= markFlag; }
    //constexpr void setBlocked()     { m_flags |= blockedFlag; }

    constexpr void setInvalid()      { m_flags |= invalidFlag; }
    constexpr void clearInvalid()    { m_flags &= ~invalidFlag; }

    static constexpr uint16_t sourceFlag    = 1;
    static constexpr uint16_t targetFlag    = 2;
    static constexpr uint16_t reachedFlag   = 4;
    static constexpr uint16_t invalidFlag   = 8;
};

/** Global router grid */
class Grid
{
public:

    /** create a grid of a certain size (in grid cell units) */
    Grid(const ChipDB::Coord64 width, const ChipDB::Coord64 height);

    [[nodiscard]] constexpr auto width() const {return m_width; }
    [[nodiscard]] constexpr auto height() const {return m_height; }

    [[nodiscard]] const GCell& at(const GCellCoordType x, const GCellCoordType y) const;
    [[nodiscard]] GCell& at(const GCellCoordType x, const GCellCoordType y);
    [[nodiscard]] const GCell& at(const GCellCoord &p) const;
    [[nodiscard]] GCell& at(const GCellCoord &p);

    GCellCoord toGridCoord(const ChipDB::Coord64 &p1) const noexcept;

protected:
    ChipDB::Coord64 m_cellSize{0,0};

    GCellCoordType m_width{0};
    GCellCoordType m_height{0};

    std::vector<GCell> m_grid;

    static GCell m_invalidCell;
};

};