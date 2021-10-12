#include <cmath>
#include "common/logging.h"
#include "region.h"

static double roundUp(double v, double resolution)
{
    return std::ceil(v / resolution) * resolution;
}

ChipDB::Region* ChipDB::createRegion(
    float   aspectRatio,    /// width / height
    int64_t minCellWidth, 
    int64_t rowHeight, 
    int64_t rowDistance,
    int64_t totalCellWidth)
{
    auto effectiveRowHeight = rowHeight + rowDistance;

    //FIXME: adjust for fill rate
    double effectiveCellArea = static_cast<double>(totalCellWidth)*static_cast<double>(effectiveRowHeight);
    double rowWidthDouble = sqrt(aspectRatio * effectiveCellArea);

    // round up to multiple of minCellWidth
    auto rowWidth = static_cast<int64_t>(roundUp(rowWidthDouble,  minCellWidth));
    
    double rowHeightDouble = effectiveCellArea / static_cast<double>(rowWidth);
    auto rowCount = static_cast<int64_t>(std::ceil(rowHeightDouble / effectiveRowHeight));

    auto region = new Region();
    region->m_rows.resize(rowCount);
    region->m_rect.setSize({rowWidth, (rowCount-1)*effectiveRowHeight + rowHeight});

    doLog(LOG_VERBOSE, "createRegion: %ld x %ld - #rows %ld\n", region->m_rect.width(), region->m_rect.height(),
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

    return region;
}
