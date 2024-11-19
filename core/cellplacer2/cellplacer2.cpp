// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cassert>

#include <thread>
#include <fstream>
#include "common/logging.h"
#include "database/database.h"
#include "cellplacer2.h"
#include "../cellplacer/rowlegalizer.h"

using namespace LunaCore::CellPlacer2;

/** assign each instance/gate a row in the quadratic placement matrix */
void Placer::mapGatesToMatrixRows(const ChipDB::Netlist &netlist,
    const PlacementRegion &r,
    GateToRowContainer &gate2Row)
{
    RowIndex rowCounter = 0;
    gate2Row.clear();

    for(auto gateId : r.m_gatesInRegion)
    {
        auto gate = netlist.m_instances.atRaw(gateId);
        if (!gate->isFixed())
        {
            gate2Row[gateId] = rowCounter++;
        }
    }
}

Placer::RowIndex Placer::findRowOfGate(const GateToRowContainer &gate2Row,
    const GateId gateId) const noexcept
{
    auto iter = gate2Row.find(gateId);
    if (iter == gate2Row.end())
    {
        return -1;
    }
    return iter->second;
}

void Placer::placeRegion(ChipDB::Netlist &netlist, PlacementRegion &region)
{
    GateToRowContainer gates2Row;

    auto &gates = netlist.m_instances;
    auto &nets  = netlist.m_nets;

    mapGatesToMatrixRows(netlist, region, gates2Row);

    auto Nrows = gates2Row.size();

    Algebra::SparseMatrix<float> Amat(Nrows);

    Algebra::Vector<float> Bvec_x(Nrows);
    Algebra::Vector<float> Bvec_y(Nrows);

    Bvec_x.zero();
    Bvec_y.zero();

    std::size_t fixups = 0;
    for(auto row : gates2Row)
    {
        auto srcGateId = row.first;
        auto rowIndex  = row.second;

        auto &srcGate = gates.atRef(srcGateId);

        std::size_t pinIndex = 0;
        for(auto netId : srcGate.connections())
        {
            // skip power and ground pins
            auto const pin = srcGate.getPin(pinIndex);
            if ((pin.m_pinInfo) && (pin.m_pinInfo->isPGPin()))
            {
                pinIndex++;
                continue;
            }

            if (netId == ChipDB::ObjectNotFound)
            {
                Logging::logWarning("Net left unconnected on instance %s\n", srcGate.name().c_str());
                continue;
            }

            auto const& net = netlist.m_nets.atRef(netId);

            if (net.numberOfConnections() <= 1)
            {
                Logging::logWarning("Net %s has 1 or fewer connections!\n", net.name().c_str());
                continue;
            }

            float weight = 1.0f/(net.numberOfConnections() - 1.0);
            if (net.m_isPortNet)
            {
                //weight *= 4.0f;
            }

            for(auto const& netConnect : net)
            {
                auto dstGateId = netConnect.m_instanceKey;
                if (dstGateId == srcGateId) continue;   // skip self references.

                Amat.at(rowIndex,rowIndex) += weight;   // A(row,row) += net weight

                auto const& dstGate = netlist.m_instances.atRef(dstGateId);
                auto const dstGatePos = m_gatePositions.at(dstGateId);
                if (dstGate.isFixed())
                {
                    // destination gate isn't movable -> change bvector only
                    // if the gate isn't inside the region, propagate it to
                    // the nearest region edge.
                    auto newLocation = propagate(region, dstGatePos);
                    Bvec_x[rowIndex] += weight*newLocation.m_x;
                    Bvec_y[rowIndex] += weight*newLocation.m_y;
                }
                else
                {
                    // if the destination gate is outside the region
                    // propagate the position and treat it as a fixed
                    // gate
                    if (region.contains(dstGatePos))
                    {
                        auto const colIndex = findRowOfGate(gates2Row, dstGateId);

                        // due to round-off errors, region.contains might return 'true'
                        // even if the gate isn't actually in the region.
                        // then, colIndex will be -1 and we should treat the
                        // gate as external.
                        if (colIndex == -1)
                        {
                            auto newLocation = propagate(region, dstGatePos);
                            Bvec_x[rowIndex] += weight*newLocation.m_x;
                            Bvec_y[rowIndex] += weight*newLocation.m_y;
                        }
                        else
                        {
                            // destination gate is movable -> change row
                            Amat(rowIndex, colIndex) -= weight;
                        }
                    }
                    else
                    {
                        auto newLocation = propagate(region, dstGatePos);

                        // destination gate isn't movable -> change bvector only
                        Bvec_x[rowIndex] += weight*newLocation.m_x;
                        Bvec_y[rowIndex] += weight*newLocation.m_y;
                    }
                }
            }
            pinIndex++;
        }
    }

    

    //Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Upper | Eigen::Lower> solver;
    //Eigen::SparseMatrix<double> eigenAmat(Nrows, Nrows);
    //toEigen(Amat, eigenAmat);
    //eigenAmat.makeCompressed();
    //solver.compute(eigenAmat);

    Algebra::Vector<float> xvec;
    Algebra::Vector<float> yvec;

    // if we have a small number of rows, don't use multi-threading
    // as the thread startup time will become dominant --> assumption..
    // ibm18, always multi-threading -> 30s
    // ibm18, limit 20 rows -> 25s
    // ibm18, limit 50 rows -> 25s
    //if (Nrows < 20)
    
    Algebra::CGSolver::JacobiPreconditioner<float> precond(Amat);

    Algebra::CGSolver::ComputeInfo info_x;
    Algebra::CGSolver::ComputeInfo info_y;

    if (true)
    {
        info_x = Algebra::CGSolver::solve(Amat, Bvec_x, xvec, precond);
        info_y = Algebra::CGSolver::solve(Amat, Bvec_y, yvec, precond);
    }
    else
    {
#if 0        
        std::thread worker1(
            [&solver, &Bvec_x, &xvec]()
            {
                xvec = solver.solve(Bvec_x);
            }
        );
        std::thread worker2(
            [&solver, &Bvec_y, &yvec]()
            {
                yvec = solver.solve(Bvec_y);
            }
        );
        worker1.join();
        worker2.join();
#endif
    }

    // check if all gates are within the region
    const float absTol = 0.1f;
    for(auto row : gates2Row)
    {
        auto gateId = row.first;
        auto rowId  = row.second;
        auto const& Gate = netlist.m_instances.atRef(gateId);

        auto const oldGateLocation = m_gatePositions.at(gateId);

        auto const newGateLocation = PointF{static_cast<float>(xvec[rowId]), static_cast<float>(yvec[rowId])};

        if (!region.contains(newGateLocation, absTol))
        {
            std::cout << "Gate: (id=" << gateId << ") old pos: " << oldGateLocation << " new pos: " << newGateLocation << " Region: " << region << "\n";
            //std::cout << "A matrix row: " << Amat.at(rowId);
            std::cout << "B vec x     : " << Bvec_x[rowId] << "\n";
            std::cout << "B vec y     : " << Bvec_y[rowId] << "\n";
            std::cout << "rowId       : " << rowId << "\n";
#if 0
            std::cout << "Amat        : " << Amat << "\n";
            std::cout << "B vec x :     " << Bvec_x << "\n";
            std::cout << "B vec y :     " << Bvec_y << "\n";
#endif
        }
        assert(!Gate.isFixed());
    }

    // update gate/instance locations
    for(auto row : gates2Row)
    {
        auto gateId = row.first;
        auto rowId  = row.second;
        auto const& Gate = netlist.m_instances.atRef(gateId);

        auto newGateLocation = PointF{static_cast<float>(xvec[rowId]), static_cast<float>(yvec[rowId])};
        //assert(region.contains(newGateLocation, absTol));

        if (!region.contains(newGateLocation))
        {
            fixups++;
            newGateLocation = propagate(region, newGateLocation);
        }

        m_gatePositions.at(gateId) = newGateLocation;
        //Gate.setCenter(newGateLocation.toCoord64());
    }

#if 0
    std::ofstream ofile("cellplacer2_pos.txt");
    std::size_t index = 0;
    for(auto gateId : region.m_gatesInRegion)
    {
        ofile << "Gate " << gateId << " pos: " << m_gatePositions.at(gateId) << "\n";
    }
    ofile.close();
#endif

    // FIXME: check this in the future:
    //        is this still relevant?
    //if (fixups != 0) std::cout << "  Number of gate fixups: " << fixups << "\n";
}

void Placer::populateGatePositions(const ChipDB::Netlist &netlist, ChipDB::Floorplan &floorplan)
{
    m_gatePositions.clear();
    m_gatePositions.reserve(netlist.m_instances.size());

    for(auto const& insKeyPair : netlist.m_instances)
    {
        // We have to make sure the unplaced gates/instances are
        // within the region so the placer doesn't flag them
        // as pseud0 terminals.
        // The placed gates/instances should remain where they are.
        assert(insKeyPair.isValid());
        if (insKeyPair->isFixed())
        {
            // keep fixed gates/instances where they are
            m_gatePositions[insKeyPair.key()] = insKeyPair->getCenter();
        }
        else
        {
            // move all placable gates/instances to the center of the
            // region
            m_gatePositions[insKeyPair.key()] = floorplan.coreRect().center();
        }
    }
}

bool Placer::place(ChipDB::Netlist &netlist,
    ChipDB::Floorplan &floorplan,
    std::size_t maxLevels, std::size_t minInstances)
{
    // sanity checks
    if (floorplan.minimumCellSize().isNullSize())
    {
        Logging::logError("Placer: minimum cell size has not been defined for the core area!\n");
        return false;
    }

    if (floorplan.rows().size() == 0)
    {
        Logging::logError("Placer: no row have been defined in the floorplan!\n");
        return false;
    }

    // report utilization factor
    const double nm2um = 1.0e-3;
    double totalCellArea = LunaCore::NetlistTools::calcTotalCellArea(netlist);
    auto regionSize      = floorplan.coreSize();
    double regionArea    = static_cast<double>(regionSize.m_x)*nm2um *
        static_cast<double>(regionSize.m_y)*nm2um;

    auto utilization = static_cast<float>(totalCellArea / regionArea);
    Logging::logInfo("Core utilization is %f percent\n", utilization*100.0f);

    m_maxLevels = maxLevels;
    m_minInstancesInRegion = minInstances;

    populateGatePositions(netlist, floorplan);

    std::deque<std::unique_ptr<PlacementRegion>> placementRegions;

    auto &placementRegion = placementRegions.emplace_back(std::make_unique<PlacementRegion>());

    placementRegion->m_rect = floorplan.coreRect();
    for(auto insKeyObjPair : netlist.m_instances)
    {
        if (!insKeyObjPair->isFixed())
        {
            placementRegion->m_gatesInRegion.push_back(insKeyObjPair.key());
        }
    }

    cycle(netlist, placementRegions);

    // write back the new positions of placed gates
    for(auto gateIdAndPos : m_gatePositions)
    {
        const auto gateId = gateIdAndPos.first;
        const auto gateCenterPos = gateIdAndPos.second;
        if (!netlist.m_instances.at(gateId)->isFixed())
        {
            auto newLocation = gateCenterPos.toCoord64();
            Logging::logVerbose("Ins %s -> pos %d,%d\n", netlist.m_instances.at(gateId)->name().c_str(),
                newLocation.m_x, newLocation.m_y);
            netlist.m_instances.at(gateId)->setCenter(newLocation);
            netlist.m_instances.at(gateId)->m_placementInfo = ChipDB::PlacementInfo::PLACED;
        }
    }

    Logging::logInfo("Running row legalizer\n");

    // legalise the cells
    LunaCore::Legalizer cellLegalizer;
    if (!cellLegalizer.legalize(floorplan, netlist))
    {
        return false;
    }

    auto hpwl = LunaCore::NetlistTools::calcHPWL(netlist);
    Logging::logInfo("HPWL = %f *1e6 nm\n", hpwl / 1.0e6);

    Logging::logInfo("Placement done\n");
    //TODO: end-case placement

    return true;
}

void Placer::cycle(ChipDB::Netlist &netlist, std::deque<std::unique_ptr<PlacementRegion>> &regions)
{
    while(!regions.empty())
    {
        assert(regions.front());

        //TODO: change this, it's ugly.
        auto region = *regions.front().get();

        placeRegion(netlist, region);

        // see if we can sub-divide the region
        if ((region.m_level < m_maxLevels) && (region.m_gatesInRegion.size() >= (m_minInstancesInRegion)))
        {
            // code to determine whether we do a vertical or horizontal cut
            Direction cutDir = ((region.m_level % 2) == 0) ? Direction::VERTICAL : Direction::HORIZONTAL;

            // create new regions
            regions.emplace_back(std::make_unique<PlacementRegion>());
            auto &r1 = *(regions.back().get());
            regions.emplace_back(std::make_unique<PlacementRegion>());
            auto &r2 = *(regions.back().get());

            r1.m_level = region.m_level + 1;
            r2.m_level = region.m_level + 1;

            cutRegion(region, cutDir, r1, r2);
            sortGates(region, cutDir);

            const std::size_t threshold = region.m_gatesInRegion.size() / 2;
            std::size_t counter = 0;

#if 0
            std::ofstream ofile("placer_pos.txt");
            std::size_t index = 0;
            for(auto gateId : region.m_gatesInRegion)
            {
                index++;
                if (index == threshold)
                {
                    ofile << "**THRESHOLD**\n";
                }
                ofile << "Gate " << gateId << " pos: " << m_gatePositions.at(gateId) << "\n";
            }
            ofile.close();
#endif

            // figure out where the cut line is
            double cutPosition = 0.0f;
            auto cutGateId = region.m_gatesInRegion.at(threshold);
            auto const& cutPos = m_gatePositions.at(cutGateId);

            if (cutDir == Direction::HORIZONTAL)
            {
                Logging::logInfo("  cut position at x=%f\n", cutPos.m_x);
                cutPosition = cutPos.m_x;
            }
            else
            {
                Logging::logInfo("  cut position at y=%f\n", cutPos.m_y);
                cutPosition = cutPos.m_y;
            }

            // FIXME: distribute according to r1 and r2 capacities

            for(auto gateId : region.m_gatesInRegion)
            {
                assert(!netlist.m_instances.at(gateId)->isFixed()); // cppcheck-suppress[assertWithSideEffect]

                if (counter < threshold)
                {
                    r2.m_gatesInRegion.push_back(gateId);
                    m_gatePositions.at(gateId) = r2.center();
                }
                else
                {
                    r1.m_gatesInRegion.push_back(gateId);
                    m_gatePositions.at(gateId) = r1.center();
                }
                counter++;
            }
            assert(r1.m_gatesInRegion.size() > 0);
            assert(r2.m_gatesInRegion.size() > 0);
        }

        regions.pop_front();
    }
}

void Placer::sortGates(PlacementRegion &region, Direction dir)
{
    // sort gates according to y
    if (dir == Direction::VERTICAL)
    {
        std::sort(region.m_gatesInRegion.begin(), region.m_gatesInRegion.end(),
            [this](GateId gateId1, GateId gateId2)
            {
                return m_gatePositions.at(gateId1).m_y < m_gatePositions.at(gateId2).m_y;
            }
        );
    }
    else
    {
        // sort gates according to x
        std::sort(region.m_gatesInRegion.begin(), region.m_gatesInRegion.end(),
            [this](GateId gateId1, GateId gateId2)
            {
                return m_gatePositions.at(gateId1).m_x < m_gatePositions.at(gateId2).m_x;
            }
        );
    }
}

void Placer::cutRegion(const PlacementRegion &region, Direction dir,
    PlacementRegion &region1, PlacementRegion &region2) const
{
    region1.m_rect = region.m_rect;
    region2.m_rect = region.m_rect;

    if (dir == Direction::VERTICAL)
    {
        region1.m_rect.m_ll.m_x = region.center().m_x;
        region2.m_rect.m_ur.m_x = region1.m_rect.m_ll.m_x;
        std::cout << "Cut along vertical axis\n";
    }
    else
    {
        region1.m_rect.m_ll.m_y = region.center().m_y;
        region2.m_rect.m_ur.m_y = region1.m_rect.m_ll.m_y;
        std::cout << "Cut along horizontal axis\n";
    }
    std::cout << "  P1: (" << region1.m_rect.m_ll.m_x << "," << region1.m_rect.m_ll.m_y << ") ";
    std::cout << " W " << region1.width() << "  H " << region1.height() << "\n";
    std::cout << "  P2: (" << region2.m_rect.m_ll.m_x << "," << region2.m_rect.m_ll.m_y << ") ";
    std::cout << " W " << region2.width() << "  H " << region2.height() << "\n";
}

PointF Placer::propagate(const PlacementRegion &r, const PointF &p) const
{
    PointF result = p;
    if (result.m_x < r.m_rect.m_ll.m_x) result.m_x = r.m_rect.m_ll.m_x;
    if (result.m_x > r.m_rect.m_ur.m_x) result.m_x = r.m_rect.m_ur.m_x;
    if (result.m_y < r.m_rect.m_ll.m_y) result.m_y = r.m_rect.m_ll.m_y;
    if (result.m_y > r.m_rect.m_ur.m_y) result.m_y = r.m_rect.m_ur.m_y;
    return result;
}

std::ostream& operator<<(std::ostream &os, const LunaCore::CellPlacer2::PointF &p)
{
    os << "(" << p.m_x << "," << p.m_y << ")";
    return os;
}

std::ostream& operator<<(std::ostream &os, const LunaCore::CellPlacer2::PlacementRegion &r)
{
    os << r.m_rect;
    return os;
}
