// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <optional>
#include <utility>
#include <memory>
#include <optional>
#include "design/design.h"
#include "datatypes.h"
#include "wavefront.h"
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

    /** create a new grid, the old one is destroyed. */
    void createGrid(const GCellCoordType width, const GCellCoordType height,
        const ChipDB::Size64 &cellSize, const int64_t cellCapacity);

    /** get a raw pointer to the grid */
    const Grid* grid() const {return m_grid.get(); }

    /** set a blockage at Chip coordinates p. The underlying grid cell will be set to blocked. */
    void setBlockage(const ChipDB::Coord64 &p);

    /** route a complete net */
    [[nodiscard]] std::optional<SegmentList> routeNet(const std::vector<ChipDB::Coord64> &netNodes,
        const std::string &netName);

    /** clear the grid for a new route, capacity values remain in tact */
    void clearGridForNewRoute();

protected:
    /** route a two-point connection between p1 and p2. Used cells are marked so they can be extracted later. */
    [[nodiscard]] std::optional<SegmentList> routeTwoPointRoute(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);

    /** follow the mark path in the bitmap and recover the routing. */
    void updateCapacity(const SegmentList &segments) const;

    bool addWavefrontCell(
        Wavefront &wavefront,
        const GCellCoord &pos,
        PathCostType newCost,
        Predecessor pred);

    /** generic LEE maze router cost function. */
    [[nodiscard]] std::optional<PathCostType> calcGridCost(const WavefrontItem &from, const GCellCoord &to, Predecessor expandDirection);

    /** A more depth-first oriented / directed maze router cost function. */
    [[nodiscard]] std::optional<PathCostType> calcGridCostDirected(const WavefrontItem &from, const GCellCoord &to, const GCellCoord &destination, Predecessor expandDirection);

    std::unique_ptr<Grid> m_grid;
};

};