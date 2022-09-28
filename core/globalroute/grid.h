// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdint>
#include <vector>
#include <cassert>
#include "datatypes.h"
#include "import/ppm/ppmreader.h"

namespace LunaCore::GlobalRouter
{

/** global grid cell */
struct GCell
{
    uint16_t        m_capacity{0};
    uint16_t        m_flags{0};
    PathCostType    m_cost{0};

    [[nodiscard]] constexpr auto cost() const noexcept
    {
        return m_cost;
    }

    constexpr void setCost(PathCostType cost) noexcept
    {
        m_cost = cost;
    }

    /** return the predecessor flag */
    constexpr auto getPredecessor() const noexcept
    {
        auto pred = static_cast<Predecessor>(m_flags & 0xFF00);
        return pred;
    }

    /** set the predecessor flag */
    constexpr void setPredecessor(Predecessor pred) noexcept
    {
        m_flags &= 0xFF;
        m_flags |= static_cast<uint16_t>(pred);
    }

    constexpr bool isSource() const {return (m_flags & sourceFlag) != 0; }
    constexpr bool isTarget() const {return (m_flags & targetFlag) != 0; }
    constexpr bool isReached() const {return (m_flags & reachedFlag) != 0; }
    constexpr bool isBlocked() const {return (m_flags & blockedFlag) != 0; }
    constexpr bool isMarked() const {return (m_flags & markedFlag) != 0; }
    constexpr bool isValid() const {return (m_flags & invalidFlag) == 0; }

    constexpr void resetFlags()     { m_flags = 0; }
    constexpr void setReached()     { m_flags |= reachedFlag; }
    constexpr void clearReached()   { m_flags &= ~reachedFlag; }
    constexpr void setTarget()      { m_flags |= targetFlag; }
    constexpr void clearTarget()    { m_flags &= ~targetFlag; }
    constexpr void setSource()      { m_flags |= sourceFlag; }
    constexpr void clearSource()    { m_flags &= ~sourceFlag; }
    constexpr void setMark()        { m_flags |= markedFlag; }
    constexpr void clearMark()      { m_flags &= ~markedFlag; }
    constexpr void setTerminal()    { m_flags |= markedFlag; }
    constexpr void clearTerminal()  { m_flags &= ~markedFlag; }
    constexpr void setBlocked()     { m_flags |= blockedFlag; }
    constexpr void clearBlocked()   { m_flags &= ~blockedFlag; }

    constexpr void setInvalid()      { m_flags |= invalidFlag; }
    constexpr void clearInvalid()    { m_flags &= ~invalidFlag; }

    static constexpr uint16_t sourceFlag    = 1;
    static constexpr uint16_t targetFlag    = 2;
    static constexpr uint16_t reachedFlag   = 4;
    static constexpr uint16_t invalidFlag   = 8;
    static constexpr uint16_t blockedFlag   = 16;
    static constexpr uint16_t markedFlag    = 32;
    static constexpr uint16_t terminalFlag  = 64;
};

/** Global router grid */
class Grid
{
public:

    /** create a grid of a certain size (in grid cell units) */
    Grid(const GCellCoordType width, const GCellCoordType height,
        const ChipDB::Size64 &cellSize);

    [[nodiscard]] constexpr auto width() const {return m_width; }
    [[nodiscard]] constexpr auto height() const {return m_height; }

    [[nodiscard]] const GCell& at(const GCellCoordType x, const GCellCoordType y) const;
    [[nodiscard]] GCell& at(const GCellCoordType x, const GCellCoordType y);
    [[nodiscard]] const GCell& at(const GCellCoord &p) const;
    [[nodiscard]] GCell& at(const GCellCoord &p);

    [[nodiscard]] GCellCoord toGridCoord(const ChipDB::Coord64 &p1) const noexcept;
    [[nodiscard]] bool isValidGridCoord(const GCellCoord &loc) const noexcept;

    void setMaxCellCapacity(int64_t cap) noexcept { m_maxCapacity = cap; assert(cap >= 0); }
    [[nodiscard]] auto maxCellCapacity() const noexcept { return m_maxCapacity; }

    /** clears the reached flag and resets the grid cost */
    void clear();
    
    /** clears all the flags in preparation for a new route */
    void clearAll();

    [[nodiscard]] PPM::Bitmap generateBitmap() const noexcept;

    auto const& gcells() const noexcept {return m_grid; };

protected:
    int             m_maxCapacity{200};
    ChipDB::Size64  m_cellSize{0,0};

    GCellCoordType m_width{0};
    GCellCoordType m_height{0};

    std::vector<GCell> m_grid;

    mutable GCell m_invalidCell;

};

};