#include <cassert>
#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include "cellplacer2.h"

using namespace LunaCore::CellPlacer2;

bool LunaCore::CellPlacer2::place(ChipDB::Design &design)
{
    // convert topmodule netlist to 
    return false;
}

/** assign each instance/gate a row in the quadratic placement matrix */
void Placer::mapGatesToMatrixRows(const ChipDB::Netlist &netlist,
    const PlacementRegion &r,
    GateToRowContainer &gate2Row)
{
    Matrix::RowIndex rowCounter = 0;
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

Matrix::RowIndex Placer::findRowOfGate(const GateToRowContainer &gate2Row, 
    const GateId gateId) const noexcept
{
    auto iter = gate2Row.find(gateId);
    if (iter == gate2Row.end())
    {
        return -1;
    }
    return iter->second;
}

void Placer::place(const ChipDB::Netlist &netlist, PlacementRegion &region)
{
    GateToRowContainer gates2Row;

    auto &gates = netlist.m_instances;
    auto &nets  = netlist.m_nets;

    mapGatesToMatrixRows(netlist, region, gates2Row);

    auto Nrows = gates2Row.size();

    Matrix Amat(Nrows);
    
    Eigen::VectorXd Bvec_x(Nrows);
    Eigen::VectorXd Bvec_y(Nrows);

    Bvec_x.setZero();
    Bvec_y.setZero();

#if 0
    std::size_t fixups = 0;
    const double weight = 1;    // FIXME: use net weight
    for(auto row : gates2Row)
    {
        auto srcGateId = row.first;        
        auto rowIndex  = row.second;

        auto srcGate = gates.atRaw(srcGateId);

        for(auto netId : srcGate->)
        {            
            auto const& net = inspec.m_nets.at(netId);

            for(auto dstGateId : net.m_gateIDs)
            {
                if (dstGateId == srcGateId) continue;   // skip self references.

                Amat(rowIndex,rowIndex) += weight;   // A(row,row) += net weight

                auto const& dstGate = inspec.m_gates.at(dstGateId);
                if (dstGate.m_isFixed)
                {
                    // destination gate isn't movable -> change bvector only
                    // if the gate isn't inside the region, propagate it to 
                    // the nearest region edge.
                    auto newLocation = propagate(region, dstGate.m_location);
                    Bvec_x[rowIndex] += weight*newLocation.m_x;
                    Bvec_y[rowIndex] += weight*newLocation.m_y;
                }
                else
                {
                    // if the destination gate is outside the region
                    // propagate the position and treat it as a fixed
                    // gate
                    if (region.contains(dstGate.m_location))
                    {
                        auto colIndex = findRow(gates2Row, dstGateId);

                        // due to round-off errors, region.contains might return 'true'
                        // even if the gate isn't actually in the region.
                        // then, colIndex will be -1 and we should treat the
                        // gate as external.
                        if (colIndex == -1)
                        {
                            auto newLocation = propagate(region, dstGate.m_location);
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
                        auto newLocation = propagate(region, dstGate.m_location);
                        
                        // destination gate isn't movable -> change bvector only
                        Bvec_x[rowIndex] += weight*newLocation.m_x;
                        Bvec_y[rowIndex] += weight*newLocation.m_y;
                    }
                }
            }
        }
    }

    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Upper | Eigen::Lower> solver;

    Eigen::SparseMatrix<double> eigenAmat(Nrows, Nrows);

    toEigen(Amat, eigenAmat);

    eigenAmat.makeCompressed();
    solver.compute(eigenAmat);

    Eigen::VectorXd xvec;
    Eigen::VectorXd yvec;

    // if we have a small number of rows, don't use multi-threading
    // as the thread startup time will become dominant --> assumption..
    // ibm18, always multi-threading -> 30s
    // ibm18, limit 20 rows -> 25s
    // ibm18, limit 50 rows -> 25s
    if (Nrows < 20)
    //if (true)
    {
        xvec = solver.solve(Bvec_x);
        yvec = solver.solve(Bvec_y);
    }
    else
    {
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
    }

    // check if all gates are within the region
    const float absTol = 0.1f;
    for(auto row : gates2Row)
    {
        auto gateId = row.first;
        auto const& Gate = inspec.m_gates.at(gateId);
        auto rowId  = row.second;

        const Point newGateLocation = Point{static_cast<float>(xvec[rowId]), static_cast<float>(yvec[rowId])};

        if (!region.contains(newGateLocation, absTol))
        {
            std::cout << "Gate: (id=" << gateId << ") old pos: " << Gate.m_location << " new pos: " << newGateLocation << " Region: " << region << "\n";
            std::cout << "A matrix row: " << eigenAmat.row(rowId);
            std::cout << "B vec x     : " << Bvec_x[rowId] << "\n";
            std::cout << "B vec y     : " << Bvec_y[rowId] << "\n";
            std::cout << "rowId       : " << rowId << "\n";
#if 0            
            std::cout << "Amat        : " << Amat << "\n";
            std::cout << "B vec x :     " << Bvec_x << "\n";
            std::cout << "B vec y :     " << Bvec_y << "\n";
#endif            
        }
        assert(!Gate.m_isFixed);   
    }

    for(auto row : gates2Row)
    {
        auto gateId = row.first;
        auto rowId  = row.second;

        auto& Gate = inspec.m_gates.at(gateId);

        const Point newGateLocation = Point{static_cast<float>(xvec[rowId]), static_cast<float>(yvec[rowId])};
        Gate.m_location = newGateLocation;

        assert(region.contains(Gate.m_location, absTol));
        
        if (!region.contains(Gate.m_location))
        {
            fixups++;
            //std::cout << "  Fixed position of gate " << gateId << " ";
            auto newLocation = propagate(region, Gate.m_location);
            //std::cout <<" was: " << Gate.m_location << "  is: " << newLocation << "\n";

            Gate.m_location = newLocation;            
        }
    }
    if (fixups != 0) std::cout << "  Number of gate fixups: " << fixups << "\n";
#endif
    
}
