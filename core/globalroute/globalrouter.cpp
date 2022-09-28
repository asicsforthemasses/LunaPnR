// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>
#include "globalrouter.h"
#include "wavefront.h"
#include "prim.h"
#include "common/logging.h"

using namespace LunaCore;

void GlobalRouter::Router::createGrid(const GCellCoordType width, const GCellCoordType height,
    const ChipDB::Size64 &cellSize)
{
    m_grid = std::make_unique<Grid>(width, height, cellSize);
}

std::optional<GlobalRouter::TrackInfo> GlobalRouter::Router::calcNumberOfTracks(const ChipDB::Design &design,
    const std::string &siteName,
    const ChipDB::Size64 &extents) const
{
    if (design.m_techLib->getNumberOfSites() == 0)
    {
        Logging::doLog(Logging::LogType::ERROR, "determineGridCellSize: no sites defined in tech lib\n");
        return std::nullopt;
    }

    auto site = design.m_techLib->lookupSiteInfo(siteName);
    if (!site.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR,"determineGridCellSize: site %s not found\n", siteName.c_str());
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
        Logging::doLog(Logging::LogType::ERROR, "determineGridCellSize: no sites defined in tech lib\n");
        return std::nullopt;
    }

    auto site = design.m_techLib->lookupSiteInfo(siteName);
    if (!site.isValid())
    {
        Logging::doLog(Logging::LogType::ERROR,"determineGridCellSize: site %s not found\n", siteName.c_str());
        return std::nullopt;
    }

    auto minStdCellSize = site->m_size;

    if ((minStdCellSize.m_x == 0) || (minStdCellSize.m_y == 0))
    {
        Logging::doLog(Logging::LogType::ERROR,"determineGridCellSize: minimum standard cell size is ill-defined: %ld, %ld\n", 
            minStdCellSize.m_x, minStdCellSize.m_y);
        return std::nullopt;        
    }

    auto const& layers = design.m_techLib->layers();

    Logging::doLog(Logging::LogType::VERBOSE, "determineGridCellSize: minimum standard cell size is: %ld, %ld\n",
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
        Logging::doLog(Logging::LogType::ERROR, "determineGridCellSize: routes per minimum standard cell: h=%f v=%f are ill-defined.\n",
            hRoutesInMinCell, vRoutesInMinCell);        
        return std::nullopt;
    }
    else
    {
        Logging::doLog(Logging::LogType::VERBOSE, "determineGridCellSize: routes per minimum standard cell: h=%f v=%f\n",
            hRoutesInMinCell, vRoutesInMinCell);
    }

    int heightUnits = static_cast<int>(std::ceil(static_cast<float>(hRoutes / hRoutesInMinCell)));
    int widthUnits  = static_cast<int>(std::ceil(static_cast<float>(vRoutes / vRoutesInMinCell)));

    Logging::doLog(Logging::LogType::VERBOSE,"determineGridCellSize: routing grid in std cell units w=%d, h=%d\n",
        widthUnits, heightUnits);

    ChipDB::CoordType w = widthUnits * minStdCellSize.m_x;
    ChipDB::CoordType h = heightUnits * minStdCellSize.m_y;

    Logging::doLog(Logging::LogType::INFO,"Routing grid cell size: w=%d nm  h=%d nm\n",
        w, h);

    return ChipDB::Size64{w,h};
}

bool GlobalRouter::Router::routeSegment(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2)
{
    if (!m_grid)
    {
        Logging::doLog(Logging::LogType::DEBUG, "GlobalRouter::Router::routeSegment m_grid == nullptr\n");
        return false;
    }

    // convert nm coordinates into grid coordinates
    auto loc1 = m_grid->toGridCoord(p1);
    auto loc2 = m_grid->toGridCoord(p2);

    if (!m_grid->isValidGridCoord(loc1))
    {
        Logging::doLog(Logging::LogType::VERBOSE, "GlobalRouter::Router::routeSegment loc1 is invalid\n");
        return false;
    } 

    if (!m_grid->isValidGridCoord(loc2)) 
    {
        Logging::doLog(Logging::LogType::VERBOSE, "GlobalRouter::Router::routeSegment loc2 is invalid\n");
        return false;
    }

    // if the locations are the same, routing isn't necessary.
    if (loc1 == loc2)
    {
        m_grid->at(loc1).m_capacity++;
        m_grid->at(loc1).setTarget();
        return true;
    }

    Wavefront wavefront;
    WavefrontItem waveItem;
    waveItem.m_gridpos  = loc1;
    waveItem.m_pathCost = 0;
    wavefront.push(waveItem);

    m_grid->at(loc1).setMark();
    m_grid->at(loc1).setSource();
    m_grid->at(loc1).clearTarget(); // make sure we don't erroneously stop the route...
    m_grid->at(loc1).setReached();
    m_grid->at(loc2).setTarget();

    bool targetReached = false;
    std::size_t evaluations = 0;

    while(!targetReached)
    {
        if (wavefront.empty())
        {
            // no path found!
            Logging::doLog(Logging::LogType::VERBOSE, "  maze: path not found\n");
            return false;
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
        if (gcell.isValid() && gcell.isTarget())
        {
            targetReached = true;

            if (evaluations < 5)
            {
                std::cout << "!@#!@#\n";
            }

            m_grid->clear();

            // backtrack from target
            auto backtrackPos = minCostPos;
            bool doBacktrack = true;
            while(doBacktrack)
            {
                auto const& gridCell = m_grid->at(backtrackPos);

                // exit at the end of the track
                if (gridCell.isSource())
                {
                    m_grid->at(backtrackPos).clearSource();
                    doBacktrack = false;
                    continue;
                }

                // mark all the cells on the new path
                // as a target to terminate the next segment
                m_grid->at(backtrackPos).setMark();
                m_grid->at(backtrackPos).clearSource();
                m_grid->at(backtrackPos).clearTarget();
                m_grid->at(backtrackPos).m_capacity++;

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

#if 0
            // FIXME: horrible code!
            // put all the sources on the wavefront
            for(int y=0; y<m_grid->height(); y++)
            {
                for(int x=0; x<m_grid->width(); x++)
                {
                    if (m_grid.at(x,y).isSource())
                    {
                        wavefront.push(WavefrontItem{grid.at(x,y).getPredecessor(), GCellCoord{x,y}, 0 /* cost */});
                    }
                }
            }
#endif

        }
        else
        {
            // ******************************************************************************************
            // ** NORTH/SOUTH **
            // ******************************************************************************************        

            auto northPos = north(minCostPos);
            if (m_grid->isValidGridCoord(northPos))
            {
                auto newCost  = calcGridCostDirected(minCostItem, northPos, loc2, GlobalRouter::Predecessor::South);
                if (newCost)
                    addWavefrontCell(wavefront, northPos, newCost.value(), GlobalRouter::Predecessor::South);
            }

            auto southPos = south(minCostPos);
            if (m_grid->isValidGridCoord(southPos))
            {
                auto newCost = calcGridCostDirected(minCostItem, southPos, loc2, GlobalRouter::Predecessor::North);
                if (newCost)
                    addWavefrontCell(wavefront, southPos, newCost.value(), GlobalRouter::Predecessor::North);
            }

            // ******************************************************************************************
            // ** EAST/WEST **
            // ******************************************************************************************        

            auto eastPos = east(minCostPos);
            if (m_grid->isValidGridCoord(eastPos))
            {
                auto newCost = calcGridCostDirected(minCostItem, eastPos, loc2, GlobalRouter::Predecessor::West);
                if (newCost)
                    addWavefrontCell(wavefront, eastPos, newCost.value(), GlobalRouter::Predecessor::West);
            }

            auto westPos = west(minCostPos);
            if (m_grid->isValidGridCoord(westPos))
            {
                auto newCost = calcGridCostDirected(minCostItem, westPos, loc2, GlobalRouter::Predecessor::East);
                if (newCost)
                    addWavefrontCell(wavefront, westPos, newCost.value(), GlobalRouter::Predecessor::East);
            }
        }
    }

    Logging::doLog(Logging::LogType::VERBOSE, "  maze evaluations: %lu\n", evaluations);

    return true;
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

bool GlobalRouter::Router::route(const ChipDB::Coord64 &p1, const ChipDB::Coord64 &p2)
{
    return routeSegment(p1, p2);
}

bool GlobalRouter::Router::routeNet(const std::vector<ChipDB::Coord64> &netNodes)
{
    if (!m_grid)
    {
        Logging::doLog(Logging::LogType::ERROR, "GlobalRouter::Router::routeNet grid is nullptr - createGrid wasn't called.\n");
        return false;
    }

    auto tree = LunaCore::Prim::prim(netNodes);

    if (tree.size() != netNodes.size())
    {
        Logging::doLog(Logging::LogType::ERROR, "GlobalRouter::Prim didn't return enough nodes (expected %lu but got %lu).\n", netNodes.size(), tree.size());
        return false;        
    }

    m_grid->clearAll();
    for(auto const& treeNode : tree)
    {
        auto p1 = treeNode.m_pos;
        for(auto const& edge : treeNode.m_edges)
        {
            auto p2 = edge.m_pos;
            auto result = routeSegment(p1,p2);
            if (!result) 
            {
                Logging::doLog(Logging::LogType::ERROR,"GlobalRouter::Router::routeNet could not complete route\n");
                return false;
            }
        }
    }
    return true;
}

void GlobalRouter::Router::clearGrid()
{
    if (m_grid) m_grid->clearAll();
}

void GlobalRouter::Router::setBlockage(const ChipDB::Coord64 &p)
{
    if (m_grid)
    {
        auto pos = m_grid->toGridCoord(p);
        m_grid->at(pos).setBlocked();
    }
}