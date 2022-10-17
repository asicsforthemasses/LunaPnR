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
        const ChipDB::Size64 &cellSize, const int64_t cellCapacity);

    /** get a raw pointer to the grid */
    const Grid* grid() const {return m_grid.get(); }

    /** set a blockage at Chip coordinates p. The underlying grid cell will be set to blocked. */
    void setBlockage(const ChipDB::Coord64 &p);

#if 0
    /** route two points - this function is mainly for testing. */
    [[nodiscard]] bool route(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);
#endif

    struct NetRouteResult
    {
        SegmentList m_segList;      ///< list of segments representing the routed net.
        bool        m_ok{false};

        [[nodiscard]] auto const& segments() const noexcept
        {
            return m_segList.m_segments;
        }

    };

    /** route a complete net */
    [[nodiscard]] NetRouteResult routeNet(const std::vector<ChipDB::Coord64> &netNodes,
        const std::string &netName);

    /** clear the grid for a new route, capacity values remain in tact */
    void clearGridForNewRoute();

protected:
    /** route a two-point connection between p1 and p2. Used cells are marked so they can be extracted later. */
    [[nodiscard]] bool routeTwoPointRoute(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);

    /** route a single track segment from point p1 to point p2 
     *  and update the grid capacity.
    */
    //[[nodiscard]] bool ext(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2);

    /** follow the mark path in the bitmap and recover the routing.
    */
    NetRouteResult generateSegmentTreeAndUpdateCapacity(const ChipDB::Coord64 &start) const;

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