// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <chrono>
#include <sstream>
#include "globalroute.h"
#include "lunacore.h"

void Tasks::GlobalRoute::execute(GUI::Database &database, ProgressCallback callback)
{
    m_status.store(Status::RUNNING);

    auto topModule = database.design().getTopModule();

    if (!topModule)
    {
        error("Top module not set\n");
        return;
    }    

    if (!topModule->m_netlist)
    {
        error("Top module has no netlist\n");
        return;        
    }

    auto netlist = topModule->m_netlist;

    // FIXME: for now use the first region to global route the top module
    // check there is a valid floorplan
    if (database.floorplan()->regionCount() == 0)
    {
        error("No regions defined in floorplan!\n");
        return;
    }

    auto regionIter = database.floorplan()->begin();
    auto region = *regionIter;

    std::stringstream ss;
    ss << "Routing region: " << region->name() << "\n";

    info(ss.str());

    // determining GCell size
    auto site = database.techLib()->lookupSiteInfo(region->site());
    if (!site.isValid())
    {
        error("Site name of region isn't valid!\n");
        return;
    }

    LunaCore::GlobalRouter::Router grouter;    

    auto minCellSize = site->m_size;
    if ((minCellSize.m_x <= 0) || (minCellSize.m_y <= 0))
    {
        std::stringstream ss;
        ss << "Minimum cell size (" << minCellSize.m_x << " by " << minCellSize.m_y << " nm) of specified region site is invalid!\n";
        error(ss.str());
        return;
    }

    auto gcellSize = grouter.determineGridCellSize(database.design(), site->name(), 100, 100);
    if (!gcellSize.has_value())
    {
        error("Could not determine GCell size!\n");
        return;        
    }

    ss.str("");
    ss << "GCell size = " << gcellSize->m_x << " by " << gcellSize->m_y << " nm\n";
    info(ss.str());

    // calculate actual number of tracks in the GCell
    auto trackInfo = grouter.calcNumberOfTracks(database.design(), site->name(), gcellSize.value());
    if (!trackInfo)
    {
        error("Could not determine the number of tracks in a GCell!\n");
        return;
    }

    ss.str("");
    ss << "GCell tracks: H=" << trackInfo->horizontal << " V=" << trackInfo->vertical << "\n";
    info(ss.str());

    auto routingCapacity = trackInfo->horizontal + trackInfo->vertical;

    // HACK: check the extents of the instances to determine the
    // routable area.
    // we really should define the DIE area first..

    ChipDB::Coord64 dieArea{0,0};
    for(auto const insKeyPair : netlist->m_instances)
    {
        if (!insKeyPair->isPlaced())
        {
            std::stringstream ss;
            ss << "Instance " << insKeyPair->name() << " has not been placed!\n";
            error(ss.str());
            return;
        }

        const auto insRect = insKeyPair->rect();
        dieArea.m_x = std::max(dieArea.m_x, insRect.right());
        dieArea.m_y = std::max(dieArea.m_y, insRect.top());
    }

    dieArea.m_x += 1000;
    dieArea.m_y += 1000;

    ss.str("");
    ss << "Approximate die area = " << dieArea << " nm\n";
    info(ss.str());

    // FIXME: without handling the region offset
    // we're making the grid larger than we're supposed to
    // and we should include blockages around the boundary
    // of the routable area too!
    auto gridWidth  = 1+dieArea.m_x / gcellSize->m_x;
    auto gridHeight = 1+dieArea.m_y / gcellSize->m_y;

    ss.str("");
    ss << "Grid size = " << gridWidth << " by " << gridHeight << " cells\n";
    info(ss.str());

    grouter.createGrid(gridWidth, gridHeight, gcellSize.value(), routingCapacity);

    // route all the nets
    info("Routing nets...\n");

    // help to update the GUI ..
    std::this_thread::yield();

    // sort nets according to the number of nodes
    const std::size_t totalNets = netlist->m_nets.size();
    std::vector<ChipDB::ObjectKey> netKeys(totalNets);

    std::size_t idx = 0;
    for(auto const netKeyPair : netlist->m_nets)
    {
        netKeys.at(idx++) = netKeyPair.key();
    }

    std::sort(netKeys.begin(), netKeys.end(), 
        [&netlist](auto const &key1, auto const &key2)
        {
            return netlist->m_nets.at(key1)->numberOfConnections() < netlist->m_nets.at(key2)->numberOfConnections();
        }
    );

    auto logLevel = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::INFO);

    std::size_t netsRouted  = 0;
    for(auto const netKey : netKeys)
    {
        auto net = netlist->m_nets.at(netKey);

        std::size_t index = 0;
        std::vector<ChipDB::Coord64> netNodes;
        netNodes.resize(net->numberOfConnections());

        // write locations of all the terminals
        for(auto netConnect : *net)
        {
            auto ins = netlist->lookupInstance(netConnect.m_instanceKey);
            if (!ins->isPlaced())
            {
                std::stringstream ss;
                ss << "  instance " << ins->name() << " has no location / is not placed.!\n";
                error(ss.str());
                return;
            }

            netNodes.at(index++) = ins->m_pos;
        }

        auto segList = grouter.routeNet(netNodes, net->name());
        if (!segList.m_ok)
        {   
            auto debugBitmap = grouter.grid()->generateCapacityBitmap();
            LunaCore::PPM::write("globalroutegrid_fail.ppm", debugBitmap);

            std::stringstream ss;
            ss << "  routed " << netsRouted << " of " << totalNets << " nets\n";
            info(ss.str());

            error("Routing failed!\n");
            Logging::setLogLevel(logLevel);
            return;
        }
        netsRouted++;
    }

    ss.str("");
    ss << "  routed " << netsRouted << " of " << totalNets << " nets\n";
    info(ss.str());

    auto debugBitmap = grouter.grid()->generateCapacityBitmap();
    LunaCore::PPM::write("globalroutegrid_ok.ppm", debugBitmap);

    info("Routing complete!\n");
    Logging::setLogLevel(logLevel);
    done();
}
