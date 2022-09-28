// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
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
        const ChipDB::Size64 &cellSize);

    /** get a raw pointer to the grid */
    const Grid* grid() const {return m_grid.get(); }

    /** set a blockage at Chip coordinates p. The underlying grid cell will be set to blocked. */
    void setBlockage(const ChipDB::Coord64 &p);

    /** route two points - mainly for testing. */
    [[nodiscard]] bool route(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);

    /** route a complete net */
    [[nodiscard]] bool routeNet(const std::vector<ChipDB::Coord64> &netNodes);

    /** clear the grid for a new route */
    void clearGrid();

protected:
    /** route a single track segment from point p1 to point p2 
     *  and update the grid capacity.
    */
    [[nodiscard]] bool routeSegment(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);

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