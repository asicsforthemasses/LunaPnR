// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>
#include "globalrouter.h"
#include "wavefront.h"
#include "common/logging.h"

using namespace LunaCore;

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
        return false;
    }

    // convert nm coordinates into grid coordinates
    auto loc1 = m_grid->toGridCoord(p1);
    auto loc2 = m_grid->toGridCoord(p2);

    // if the locations are the same, routing isn't necessary.
    if (loc1 == loc2)
    {
        m_grid->at(loc1).m_capacity++;
        return true;
    }

    Wavefront wavefront;
    WavefrontItem waveItem;
    waveItem.m_gridpos  = loc1;
    waveItem.m_pathCost = 0;
    wavefront.push(waveItem);

    m_grid->at(loc2).setTarget();

    return true;
}