// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>
#include <queue>
#include "globalrouter.h"
#include "wavefront.h"
#include "prim.h"
#include "common/logging.h"

using namespace LunaCore;

void GlobalRouter::Router::createGrid(const GCellCoordType width, const GCellCoordType height,
    const ChipDB::Size64 &cellSize, const int64_t cellCapacity)
{
    m_grid = std::make_unique<Grid>(width, height, cellSize);
    m_grid->setMaxCellCapacity(cellCapacity);
}

std::optional<GlobalRouter::TrackInfo> GlobalRouter::Router::calcNumberOfTracks(const ChipDB::Design &design,
    const std::string &siteName,
    const ChipDB::Size64 &extents) const
{
    if (design.m_techLib->getNumberOfSites() == 0)
    {
        Logging::logError("determineGridCellSize: no sites defined in tech lib\n");
        return std::nullopt;
    }

    auto site = design.m_techLib->lookupSiteInfo(siteName);
    if (!site.isValid())
    {
        Logging::logError("determineGridCellSize: site %s not found\n", siteName.c_str());
        return std::nullopt;
    }

    auto const& layers = design.m_techLib->layers();

    float horizontalTracks = 0.0;  // number of horizontal routes
    float verticalTracks   = 0.0;

    for(auto const layer : layers)
    {
        if (!layer.isValid()) continue;

        if (layer->m_type == ChipDB::LayerType::ROUTING)
        {
            if (layer->m_dir == ChipDB::LayerDirection::VERTICAL)
            {
                verticalTracks += static_cast<float>(extents.m_x) / static_cast<float>(layer->m_pitch.m_x);
            }
            else if (layer->m_dir == ChipDB::LayerDirection::HORIZONTAL)
            {
                horizontalTracks += static_cast<float>(extents.m_y) / static_cast<float>(layer->m_pitch.m_y);
            }
        }
    }

    auto hTracks = static_cast<int>(std::floor(horizontalTracks));
    auto vTracks = static_cast<int>(std::floor(verticalTracks));

    return TrackInfo{hTracks, vTracks};
}

std::optional<ChipDB::Size64> GlobalRouter::Router::determineGridCellSize(const ChipDB::Design &design,
    const std::string &siteName,
    int hRoutes, int vRoutes) const
{
    if (design.m_techLib->getNumberOfSites() == 0)
    {
        Logging::logError("determineGridCellSize: no sites defined in tech lib\n");
        return std::nullopt;
    }

    auto site = design.m_techLib->lookupSiteInfo(siteName);
    if (!site.isValid())
    {
        Logging::logError("determineGridCellSize: site %s not found\n", siteName.c_str());
        return std::nullopt;
    }

    auto minStdCellSize = site->m_size;

    if ((minStdCellSize.m_x == 0) || (minStdCellSize.m_y == 0))
    {
        Logging::logError("determineGridCellSize: minimum standard cell size is ill-defined: %ld, %ld\n",
            minStdCellSize.m_x, minStdCellSize.m_y);
        return std::nullopt;
    }

    auto const& layers = design.m_techLib->layers();

    Logging::logVerbose("determineGridCellSize: minimum standard cell size is: %ld, %ld\n",
        minStdCellSize.m_x, minStdCellSize.m_y);

    float hRoutesInMinCell = 0.0;  // number of horizontal routes per min std cell size
    float vRoutesInMinCell = 0.0;

    for(auto const layer : layers)
    {
        if (!layer.isValid()) continue;

        if (layer->m_type == ChipDB::LayerType::ROUTING)
        {
            if (layer->m_dir == ChipDB::LayerDirection::HORIZONTAL)
            {
                hRoutesInMinCell += static_cast<float>(minStdCellSize.m_y) / static_cast<float>(layer->m_pitch.m_x);
            }
            else if (layer->m_dir == ChipDB::LayerDirection::VERTICAL)
            {
                vRoutesInMinCell += static_cast<float>(minStdCellSize.m_x) / static_cast<float>(layer->m_pitch.m_y);
            }
        }
    }

    if ((hRoutesInMinCell <= 0.0) || (vRoutesInMinCell <= 0.0))
    {
        Logging::logError("determineGridCellSize: routes per minimum standard cell: h=%f v=%f are ill-defined.\n",
            hRoutesInMinCell, vRoutesInMinCell);
        return std::nullopt;
    }
    else
    {
        Logging::logVerbose("determineGridCellSize: routes per minimum standard cell: h=%f v=%f\n",
            hRoutesInMinCell, vRoutesInMinCell);
    }

    int heightUnits = static_cast<int>(std::ceil(static_cast<float>(hRoutes / hRoutesInMinCell)));
    int widthUnits  = static_cast<int>(std::ceil(static_cast<float>(vRoutes / vRoutesInMinCell)));

    Logging::logVerbose("determineGridCellSize: routing grid in std cell units w=%d, h=%d\n",
        widthUnits, heightUnits);

    ChipDB::CoordType w = widthUnits * minStdCellSize.m_x;
    ChipDB::CoordType h = heightUnits * minStdCellSize.m_y;

    Logging::logInfo("Routing grid cell size: w=%d nm  h=%d nm\n",
        w, h);

    return ChipDB::Size64{w,h};
}

std::optional<LunaCore::GlobalRouter::SegmentList> GlobalRouter::Router::routeTwoPointRoute(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2)
{
    if (!m_grid)
    {
        Logging::logDebug("GlobalRouter::Router::routeSegment m_grid == nullptr\n");
        return std::nullopt;
    }

    // convert nm coordinates into grid coordinates
    auto sourceLoc = m_grid->toGridCoord(p1);
    auto targetLoc = m_grid->toGridCoord(p2);

    if (!m_grid->isValidGridCoord(sourceLoc))
    {
        Logging::logVerbose("GlobalRouter::Router::routeSegment loc1 (%lu,%lu) is invalid\n",
            sourceLoc.m_x, sourceLoc.m_y);
        return std::nullopt;
    }

    if (!m_grid->isValidGridCoord(targetLoc))
    {
        Logging::logVerbose("GlobalRouter::Router::routeSegment loc2 (%lu,%lu) is invalid\n",
            targetLoc.m_x, targetLoc.m_y);
        return std::nullopt;
    }

    // if the locations are the same, routing isn't necessary.
    if (sourceLoc == targetLoc)
    {
        SegmentList segments;
        auto seg = segments.createNewSegment(sourceLoc, Direction::East /* we can choose any direction*/, nullptr);
        seg->m_length = 0; // start and end are the same!

        return std::move(segments);
    }

    Wavefront wavefront;
    WavefrontItem waveItem;
    waveItem.m_gridpos  = sourceLoc;
    waveItem.m_pathCost = 0;
    wavefront.push(waveItem);

    m_grid->at(sourceLoc).setMark();
    m_grid->at(sourceLoc).setSource();
    m_grid->at(sourceLoc).clearTarget(); // make sure we don't erroneously stop the route...
    m_grid->at(sourceLoc).setReached();

    m_grid->at(targetLoc).setTarget();

    bool targetReached = false;
    std::size_t evaluations = 0;

    while(true)
    {
        if (wavefront.empty())
        {
            // no path found!
            Logging::logVerbose("  maze: path not found\n");
            return std::nullopt;
        }

        evaluations++;

        // get the lowest cost item from the wavefront
        const auto minCostItem = wavefront.getLowestCostItem();
        const auto minCostPos  = minCostItem.m_gridpos;

        wavefront.pop();

        m_grid->at(minCostPos).setReached();

        // if the cost of the wavefront item is larger than
        // the one in the grid, skip the update.
        auto cellCost = m_grid->at(minCostPos).cost();
        if (cellCost <= minCostItem.m_pathCost)
        {
            //std::cout << "skipped\n";
            continue;
        }

        m_grid->at(minCostPos).setPredecessor(minCostItem.m_pred);
        m_grid->at(minCostPos).setCost(minCostItem.m_pathCost);

        auto const& gcell = m_grid->at(minCostPos);
        if (gcell.isValid() && (minCostPos == targetLoc))
        {
            // back-trace from the target to the source
            // through the predecessor flags.

            // clear the grid reached and cost info to
            // prepare for the next route.
            m_grid->clearReachedAndResetCost();
            m_grid->at(sourceLoc).clearSource();
            m_grid->at(targetLoc).clearTarget();

            // backtrack from target
            auto backtrackPos = minCostPos;
            bool doBacktrack = true;

            SegmentList segments;
            auto curSegment = segments.createNewSegment(backtrackPos, Direction::Undefined);

            while(doBacktrack)
            {
                assert(curSegment != nullptr);

                auto const& gridCell = m_grid->at(backtrackPos);

                // check if this is the first segment
                // if so, init the data of the segment
                if (curSegment->m_dir == Direction::Undefined)
                {
                    curSegment->m_dir = predecessorToDirection(gridCell.getPredecessor());
                    curSegment->m_length = 1;
                }

                // exit at the end of the track when we get to the source
                if (backtrackPos == sourceLoc)
                {
                    Logging::logVerbose("  maze evaluations: %lu\n", evaluations);
                    return std::move(segments);
                }

                // check if we changed direction
                // if so, create a new segment
                if (gridCell.getPredecessor() != directionToPredecessor(curSegment->m_dir))
                {
                    curSegment = segments.createNewSegment(backtrackPos,
                        predecessorToDirection(gridCell.getPredecessor()), curSegment);
                }

                curSegment->m_length++;

                // mark all the cells on the new path
                m_grid->at(backtrackPos).setMark();

                // go to the previous cell
                switch(gridCell.getPredecessor())
                {
                case GlobalRouter::Predecessor::East:
                    backtrackPos = east(backtrackPos);
                    break;
                case GlobalRouter::Predecessor::West:
                    backtrackPos = west(backtrackPos);
                    break;
                case GlobalRouter::Predecessor::North:
                    backtrackPos = north(backtrackPos);
                    break;
                case GlobalRouter::Predecessor::South:
                    backtrackPos = south(backtrackPos);
                    break;
                default:
                    doBacktrack = false;
                    break;
                };
            }
        }
        else
        {
            // ******************************************************************************************
            // ** NORTH/SOUTH **
            // ******************************************************************************************

            auto northPos = north(minCostPos);
            if (m_grid->isValidGridCoord(northPos))
            {
                auto newCost  = calcGridCostDirected(minCostItem, northPos, targetLoc, GlobalRouter::Predecessor::South);
                if (newCost)
                    addWavefrontCell(wavefront, northPos, newCost.value(), GlobalRouter::Predecessor::South);
            }

            auto southPos = south(minCostPos);
            if (m_grid->isValidGridCoord(southPos))
            {
                auto newCost = calcGridCostDirected(minCostItem, southPos, targetLoc, GlobalRouter::Predecessor::North);
                if (newCost)
                    addWavefrontCell(wavefront, southPos, newCost.value(), GlobalRouter::Predecessor::North);
            }

            // ******************************************************************************************
            // ** EAST/WEST **
            // ******************************************************************************************

            auto eastPos = east(minCostPos);
            if (m_grid->isValidGridCoord(eastPos))
            {
                auto newCost = calcGridCostDirected(minCostItem, eastPos, targetLoc, GlobalRouter::Predecessor::West);
                if (newCost)
                    addWavefrontCell(wavefront, eastPos, newCost.value(), GlobalRouter::Predecessor::West);
            }

            auto westPos = west(minCostPos);
            if (m_grid->isValidGridCoord(westPos))
            {
                auto newCost = calcGridCostDirected(minCostItem, westPos, targetLoc, GlobalRouter::Predecessor::East);
                if (newCost)
                    addWavefrontCell(wavefront, westPos, newCost.value(), GlobalRouter::Predecessor::East);
            }
        }
    }

    return std::nullopt; // error
}

bool GlobalRouter::Router::addWavefrontCell(
    Wavefront &wavefront,
    const GCellCoord &pos,
    PathCostType newCost,
    Predecessor pred)
{
    if (!m_grid) return false;

    if (!m_grid->isValidGridCoord(pos)) return false;

    // only put cells on the wavefront that haven't been reached before
    if (!m_grid->at(pos).isReached())
    {
        if (m_grid->at(pos).isBlocked())
        {
            return false;
        }

        WavefrontItem item;
        item.m_gridpos  = pos;
        item.m_pathCost = newCost;
        item.m_pred     = pred;
        wavefront.push(item);
        return true;
    }

    return false;
}

std::optional<GlobalRouter::PathCostType> GlobalRouter::Router::calcGridCost(const WavefrontItem &from, const GCellCoord &to, Predecessor expandDirection)
{
    const PathCostType cellCost    = 1;
    const PathCostType bendPenalty = 2;

    if (!m_grid) return std::nullopt;

    if (!m_grid->isValidGridCoord(to)) return std::nullopt;

    PathCostType cost = cellCost;
    if ((from.m_pred != expandDirection) && (from.m_pred != Predecessor::Undefined))
    {
        cost += bendPenalty;
    }

    auto destinationCapacity = m_grid->at(to).m_capacity;
    if (destinationCapacity >= m_grid->maxCellCapacity())
    {
        return std::nullopt;
    }

    return cost + from.m_pathCost;
}

std::optional<GlobalRouter::PathCostType> GlobalRouter::Router::calcGridCostDirected(const WavefrontItem &from, const GCellCoord &to, const GCellCoord &destination, Predecessor expandDirection)
{
    const PathCostType cellCost    = 1;
    const PathCostType bendPenalty = 2;
    const PathCostType borderSlack = 0;

    if (!m_grid) return std::nullopt;

    if (!m_grid->isValidGridCoord(to)) return std::nullopt;

    PathCostType cost = cellCost;
    if ((from.m_pred != expandDirection) && (from.m_pred != Predecessor::Undefined))
    {
        cost += bendPenalty;
    }

    auto destinationCapacity = m_grid->at(to).m_capacity;
    if (destinationCapacity >= m_grid->maxCellCapacity())
    {
        return std::nullopt;
    }

    return cost + from.m_pathCost + std::max(to.manhattanDistance(destination) - borderSlack, (int64_t)0);
}

std::optional<LunaCore::GlobalRouter::SegmentList> GlobalRouter::Router::routeNet(const std::vector<ChipDB::Coord64> &netNodes, const std::string &netName)
{
    if (!m_grid)
    {
        Logging::logError("GlobalRouter::Router::routeNet grid is nullptr - createGrid wasn't called.\n");
        return std::nullopt;
    }

    auto tree = LunaCore::Prim::prim(netNodes);

    if (tree.size() != netNodes.size())
    {
        Logging::logError("GlobalRouter::Prim didn't return enough nodes (expected %lu but got %lu).\n", netNodes.size(), tree.size());
        return std::nullopt;
    }

    SegmentList allSegments;

    m_grid->clearAllFlagsAndResetCost();
    for(auto const& treeNode : tree)
    {
        auto p1 = treeNode.m_pos;
        for(auto const& edge : treeNode.m_edges)
        {
            auto p2 = edge.m_pos;
            auto segments = routeTwoPointRoute(p1,p2);

            if (!segments)
            {
                Logging::logError("GlobalRouter::Router::routeNet could not complete route %s (%lu nodes)\n",
                    netName.c_str(), netNodes.size());
                return std::nullopt;
            }

            // TODO:
            //
            // all segments start at p1 so there will be overlap
            // remove the overlap in the segments set.

            allSegments.absorb(segments.value());
        }
    }

    updateCapacity(allSegments);

    return std::move(allSegments);
}

void GlobalRouter::Router::clearGridForNewRoute()
{
    if (m_grid) m_grid->clearAllFlagsAndResetCost();
}

void GlobalRouter::Router::setBlockage(const ChipDB::Coord64 &p)
{
    if (m_grid)
    {
        auto pos = m_grid->toGridCoord(p);
        m_grid->at(pos).setBlocked();
    }
}

void GlobalRouter::Router::updateCapacity(const SegmentList &segments) const
{
    if (!m_grid)
    {
        Logging::logError("GlobalRouter::Router::updateCapacity grid is nullptr - createGrid wasn't called.\n");
    }

    for(auto seg : segments)
    {
        assert(seg != nullptr);

        //auto gridCoord = m_grid->toGridCoord(seg->m_start);
        auto gridCoord = seg->m_start;

        auto cellCount = seg->m_length;
        while(cellCount > 0)
        {
            if (!m_grid->isValidGridCoord(gridCoord))
            {
                std::stringstream ss;
                ss << "GlobalRouter::Router::updateCapacity point " << gridCoord << " is not within the grid!";
                ss << "  segment start: " << seg->m_start << " len: " << seg->m_length << "\n";
                Logging::logError(ss.str());
                return;
            }

            // check if already visited..
            // FIXME: how about len=0; segments?
            if (!m_grid->at(gridCoord).isExtracted())
            {
                m_grid->at(gridCoord).setExtracted();
                m_grid->at(gridCoord).m_capacity++;
            }

            cellCount--;

            switch(seg->m_dir)
            {
            case Direction::East:
                gridCoord = east(gridCoord);
                break;
            case Direction::West:
                gridCoord = west(gridCoord);
                break;
            case Direction::North:
                gridCoord = north(gridCoord);
                break;
            case Direction::South:
                gridCoord = south(gridCoord);
                break;
            default:
                Logging::logError("GlobalRouter::Router::updateCapacity segment has no direction!\n");
                return;
            }
        }
    }
}
