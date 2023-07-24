// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <unordered_map>
#include "common/logging.h"
#include "cellplacer.h"
#include "qlaplacer.h"
#include "rowlegalizer.h"

using namespace LunaCore::QLAPlacer;


bool LunaCore::QLAPlacer::place(
    const ChipDB::Region  &region,
    ChipDB::Netlist &netlist,
    std::function<void(const LunaCore::QPlacer::PlacerNetlist &)> callback)
{
    double area = 0.0f;

    if (region.getMinCellSize().isNullSize())
    {
        Logging::doLog(Logging::LogType::ERROR,"Cannot place: minimum cell size is 0.\n");
        return false;
    }

    const auto regionRect = region.m_rect;

    if (region.m_rows.empty())
    {
        Logging::doLog(Logging::LogType::ERROR,"Cannot place: region has no rows\n");
        return false;
    }

    Logging::doLog(Logging::LogType::INFO, "Placing netlist in rectangle (%d,%d)-(%d,%d).\n",
        regionRect.left(), regionRect.bottom(), 
        regionRect.right(), regionRect.top());

    // check if pins have been fixed
    // and calculate total area
    for(auto ins : netlist.m_instances)
    {
        if (ins->isPin())
        {
            if (ins->m_placementInfo != ChipDB::PlacementInfo::PLACEDANDFIXED)
            {
                std::stringstream ss;
                ss << "Not all pins have been placed and fixed - for example: " << ins->name() << "\n";
                Logging::doLog(Logging::LogType::ERROR, ss);
                return false;
            }
        }

        area += ins->instanceSize().m_x * ins->instanceSize().m_y;
    }

    const auto regionArea = regionRect.width() * regionRect.height();

    if (regionArea < area)
    {
        std::stringstream ss;
        ss << "The region area (" << regionArea << ") is smaller than the total instance area (" << area << ")\n";
        Logging::doLog(Logging::LogType::ERROR, ss);
        return false;
    }

    Logging::doLog(Logging::LogType::INFO, "Utilization = %3.1f percent\n", 100.0* area / static_cast<double>(regionArea));

    auto placerNetlist = Private::createPlacerNetlist(netlist);
    
    Private::doInitialPlacement(regionRect, placerNetlist);    
    Private::updatePositions(placerNetlist, netlist);

    Logging::doLog(Logging::LogType::INFO, "Initial HPWL = %lf\n", Private::calcHPWL(placerNetlist));

    double hpwlCost    = std::numeric_limits<double>::max();
    double oldHpwlCost = std::numeric_limits<double>::max();
    size_t iterCount = 1;
    while(iterCount < 20)
    {   
        oldHpwlCost = hpwlCost;
        Private::doQuadraticB2B(placerNetlist);
                
        if (callback)
        {
            callback(placerNetlist);
        }

        hpwlCost = Private::calcHPWL(placerNetlist);
        Logging::doLog(Logging::LogType::INFO,"Iteration %d HPWL %f\n", iterCount, hpwlCost);

        if (hpwlCost < oldHpwlCost)
        {
            Private::updatePositions(placerNetlist, netlist);
        }
        else
        {
            break;
        }

        iterCount++;
    } 

    Logging::doLog(Logging::LogType::VERBOSE, "Running final legalization.\n");
    LunaCore::Legalizer legalizer;
    if (!legalizer.legalizeRegion(region, netlist))
    {
        return false;
    }

    Logging::doLog(Logging::LogType::INFO, "Placement done.\n");

    return true;
}
