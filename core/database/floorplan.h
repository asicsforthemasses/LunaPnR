// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
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
        m_io2coreMargins = margins;
    }

    [[nodiscard]] Margins64 ioMargins() const noexcept
    {
        return m_io2coreMargins;
    }

    constexpr void setCoreSize(const Size64 &coreSize) noexcept
    {
        m_coreSize = coreSize;
    }

    [[nodiscard]] constexpr auto& rows() noexcept { return m_rows; }
    [[nodiscard]] constexpr auto const& rows() const noexcept { return m_rows; }

    constexpr void setMinimumCellSize(const Size64 &minimumCellSize) noexcept
    {
        m_minimumCellSize = minimumCellSize;
    }

    [[nodiscard]] Size64 minimumCellSize() const noexcept
    {
        return m_minimumCellSize;
    }
protected:
    Size64    m_coreSize;       ///< core area
    Margins64 m_io2coreMargins; ///< margins between core and io area
    Margins64 m_ioMargins;      ///< size of IO area at top, bottom, left and right sides.

    Size64    m_minimumCellSize;

    std::vector<ChipDB::Row> m_rows;
};

};
