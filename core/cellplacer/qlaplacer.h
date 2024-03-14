// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

/*

    Quadratic look-ahead placer

*/

#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include <functional>

#include "database/database.h"
#include "common/matrix.h"
#include "qplacertypes.h"

namespace LunaCore::QLAPlacer::Private
{
    struct SolverData
    {
        //Eigen::SparseMatrix<double> m_Amat;
        LunaCore::Matrix m_Amat;
        Eigen::VectorXd m_Bvec;
    };

    struct Block
    {
        ChipDB::Rect64 m_extents;
        uint32_t       m_level;
    };

    /** create a PlacerNetlist from a ChipDB::Netlist */
    LunaCore::QPlacer::PlacerNetlist createPlacerNetlist(const ChipDB::Netlist &nl);

    /** do intial placement of cells based on a uniform random distribution */
    bool doInitialPlacement(const ChipDB::Rect64 &regionRect, LunaCore::QPlacer::PlacerNetlist &netlist);

    /** use the B2B net model and a quadratic solver to get kinda-optimal (overlapping) placements */
    bool doQuadraticB2B(LunaCore::QPlacer::PlacerNetlist &netlist);

    /** write the positions in the PlacerNetlist back to the ChipDB::Netlist */
    bool updatePositions(const LunaCore::QPlacer::PlacerNetlist &netlist, ChipDB::Netlist &nl);

    /** calculate half perimiter wire length in nm */
    double calcHPWL(const LunaCore::QPlacer::PlacerNetlist &netlist);

    void writeNetlistToSVG(std::ostream &os, const ChipDB::Rect64 &regionRect, const LunaCore::QPlacer::PlacerNetlist &netlist);

    void lookaheadLegaliser(const ChipDB::Rect64 &regionRect, LunaCore::QPlacer::PlacerNetlist &netlist);

    void doNonlinearScaling(const Block &block, LunaCore::QPlacer::PlacerNetlist &netlist);

};

namespace LunaCore::QLAPlacer
{

    /** place the module in the region rectangle.
     *  the callback is called each iteration when the positions have been updated
    */
    bool place(
        const ChipDB::Floorplan &floorplan,
        ChipDB::Netlist &netlist,
        std::function<void(const LunaCore::QPlacer::PlacerNetlist &)> callback);

};
