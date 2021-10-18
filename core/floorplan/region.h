#pragma once

#include <vector>
#include <optional>
#include "common/dbtypes.h"
#include "row.h"

namespace ChipDB
{

struct Region
{
    std::string      m_site;    ///< site name
    Rect64           m_rect;    ///< size of the region (includes halo)
    Margins64        m_halo;    ///< halo _inside_ of the region rect to keep free
    std::vector<Row> m_rows;    ///< the rows in this region

    Rect64 getPlacementRect() const;  ///< returns the inside rectangle of region where cells/rows can be placed
};

Region* createRegion(
    float   aspectRatio,
    int64_t minCellWidth, 
    int64_t CellHeight, 
    int64_t rowDistance,
    int64_t totalCellWidth,
    std::optional<Margins64> halo = std::nullopt);

};