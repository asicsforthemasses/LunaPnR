/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#include <cmath>

#include "common/logging.h"
#include "region.h"

ChipDB::Rect64 ChipDB::Region::getPlacementRect() const
{
    return m_rect.contracted(m_halo);
}

static double roundUp(double v, double resolution)
{
    return std::ceil(v / resolution) * resolution;
}

std::shared_ptr<ChipDB::Region> ChipDB::createRegion(
    float   aspectRatio,    /// width / height
    int64_t minCellWidth, 
    int64_t rowHeight, 
    int64_t rowDistance,
    int64_t totalCellWidth,
    std::optional<Margins64> halo)
{
    auto effectiveRowHeight = rowHeight + rowDistance;

    //FIXME: adjust for utilisation factor
    double effectiveCellArea = static_cast<double>(totalCellWidth)*static_cast<double>(effectiveRowHeight);
    double rowWidthDouble = sqrt(aspectRatio * effectiveCellArea);

    // round up to multiple of minCellWidth
    auto rowWidth = static_cast<int64_t>(roundUp(rowWidthDouble,  minCellWidth));
    
    double rowHeightDouble = effectiveCellArea / static_cast<double>(rowWidth);
    auto rowCount = static_cast<int64_t>(std::ceil(rowHeightDouble / effectiveRowHeight));

    auto region = std::make_shared<Region>();
    region->m_rows.resize(rowCount);
    region->m_rect.setSize({rowWidth, (rowCount-1)*effectiveRowHeight + rowHeight});

    Logging::doLog(Logging::LogType::VERBOSE, "createRegion: %ld x %ld - #rows %ld\n", region->m_rect.width(), region->m_rect.height(),
        rowCount);

    // create rows inside region
    int64_t ll_y = 0;
    for(size_t i=0; i<rowCount; i++)
    {
        region->m_rows.emplace_back();
        region->m_rows.back().m_region = region;
        region->m_rows.back().m_rect   = Rect64({0,ll_y}, {rowWidth, rowHeight});
        ll_y += effectiveRowHeight;
    }

    if (halo)
    {
        region->m_halo = *halo;
        region->m_rect.expand(*halo);
    }

    return region;
}
