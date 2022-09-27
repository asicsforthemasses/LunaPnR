// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <optional>
#include <utility>
#include <memory>
#include "datatypes.h"
#include "design/design.h"
#include "grid.h"

namespace LunaCore::GlobalRouter
{

class Router
{
public:
    Router() = default;
    
    std::optional<ChipDB::Size64> determineGridCellSize(const ChipDB::Design &design, 
        const std::string &siteName,
        int hRoutes, int vRoutes) const;

    std::optional<TrackInfo> calcNumberOfTracks(const ChipDB::Design &design,
        const std::string &siteName,
        const ChipDB::Size64 &extents) const;

protected:
    /** route a single track segment from point p1 to point p2 
     *  and update the grid capacity.
    */
    bool routeSegment(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);

    std::unique_ptr<Grid> m_grid;
};

};