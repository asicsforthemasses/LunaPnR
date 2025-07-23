// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <vector>
#include <optional>
#include <memory>
#include "dbtypes.h"
#include "row.h"

namespace ChipDB
{

/** a region is a part of the chip area that holds rows where standard ASIC cells can be placed. */
class Region
{
public:

    Region(const std::string &regionName, const std::string &siteName)
        : m_name(regionName), m_site(siteName) {}

    std::string name() const noexcept
    {
        return m_name;
    }

    std::string site() const noexcept
    {
        return m_site;
    }

    Rect64           m_rect;    ///< size of the region (includes halo)
    Margins64        m_halo;    ///< halo _inside_ of the region rect to keep free
    std::vector<Row> m_rows;    ///< the rows in this region

    [[nodiscard]] Rect64 getPlacementRect() const noexcept; ///< returns the inside rectangle of region where cells/rows can be placed
    [[nodiscard]] Size64 getPlacementSize() const noexcept; ///< returns the size (excluding halo) of the region in nm.

    [[nodiscard]] Size64 getSize() const noexcept; ///< returns the size (including halo) of the region in nm.

    [[nodiscard]] Size64 getMinCellSize() const noexcept;       ///< return the minimum cell size in nm.
    void setMinCellSize(const Size64 &minCellSize) noexcept;    ///< set the minimum cell size in nm.

protected:
    Size64           m_minCellSize; ///< minimum cell size, information from LEF SITE definition
    std::string      m_name;        ///< region name
    std::string      m_site;        ///< site name
};

[[nodiscard]] std::shared_ptr<Region> createRegion(
    const std::string &regionName,
    const std::string &siteName,
    Rect64 regionRectIncludingHalo,     /* region extents including routing halo */
    Size64 minCellSize,                 /* minimum cell size in nm */
    std::optional<Margins64> halo = std::nullopt
);

#if 0
std::shared_ptr<Region> createRegion(
    float   aspectRatio,
    int64_t minCellWidth,
    int64_t CellHeight,
    int64_t rowDistance,
    int64_t totalCellWidth,
    std::optional<Margins64> halo = std::nullopt);

#endif

};