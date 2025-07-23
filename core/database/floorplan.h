// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#pragma once

#include <optional>
#include "dbtypes.h"
#include "namedstorage.h"
#include "region.h"

namespace ChipDB
{

/**
    The floorplan holds information about the die area and core area, among other things.

*/
class Floorplan
{
public:

    void clear();

    void addListener(INamedStorageListener *listener);
    void removeListener(INamedStorageListener *listener);

    /** notify all listeners that the floorplan has changed */
    void contentsChanged() const;

    [[nodiscard]] Size64 dieSize() const noexcept;
    [[nodiscard]] Rect64 coreRect() const noexcept;

    [[nodiscard]] Size64 coreSize() const noexcept
    {
        return m_coreSize;
    }

    [[nodiscard]] Size64 cornerCellSize() const noexcept
    {
        return m_cornerCellSize;
    }

    constexpr void setIO2CoreMargins(const Margins64 &margins) noexcept
    {
        m_io2coreMargins = margins;
    }

    [[nodiscard]] Margins64 io2CoreMargins() const noexcept
    {
        return m_io2coreMargins;
    }

    constexpr void setIOMargins(const Margins64 &margins) noexcept
    {
        m_ioMargins = margins;
    }

    [[nodiscard]] Margins64 ioMargins() const noexcept
    {
        return m_ioMargins;
    }

    constexpr void setCoreSize(const Size64 &coreSize) noexcept
    {
        m_coreSize = coreSize;
    }

    constexpr void setCornerCellSize(const Size64 &cornerCellSize)
    {
        m_cornerCellSize = cornerCellSize;
        //FIXME: we might need to the IO margins
    }

    [[nodiscard]] constexpr auto& rows() noexcept { return m_rows; }
    [[nodiscard]] constexpr auto const& rows() const noexcept { return m_rows; }

    constexpr void setMinimumCellSize(const Size64 &minimumCellSize) noexcept
    {
        m_minimumCellSize = minimumCellSize;
        //FIXME: we might need to update rows
    }

    [[nodiscard]] Size64 minimumCellSize() const noexcept
    {
        return m_minimumCellSize;
    }

protected:
    Size64    m_coreSize;           ///< core area
    Margins64 m_io2coreMargins;     ///< margins between core and io area
    Margins64 m_ioMargins;          ///< size of IO area at top, bottom, left and right sides.

    Size64    m_minimumCellSize;    ///< minimum cell size for this site. needs to be set in order to create rows.
    Size64    m_cornerCellSize;     ///< size of an IO corner cell, so we can create a pad ring.

    std::vector<ChipDB::Row> m_rows;
};

};
