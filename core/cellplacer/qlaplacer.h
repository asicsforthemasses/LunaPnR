/*

    Quadratic look-ahead placer

*/

#pragma once

#include <Eigen/Core>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/IterativeLinearSolvers>

#include "common/dbtypes.h"
#include "netlist/netlist.h"
#include "qplacertypes.h"

namespace LunaCore::QLAPlacer::Private
{
    struct SolverData
    {
        Eigen::SparseMatrix<double> m_Amat;
        Eigen::VectorXd m_Bvec;
    };

    LunaCore::QPlacer::PlacerNetlist createPlacerNetlist(const ChipDB::Netlist &nl);
    bool doInitialPlacement(const ChipDB::Rect64 &regionRect, LunaCore::QPlacer::PlacerNetlist &netlist);
    bool doQuadraticB2B(LunaCore::QPlacer::PlacerNetlist &netlist);
    bool updatePositions(const LunaCore::QPlacer::PlacerNetlist &netlist, ChipDB::Netlist &nl);

    double calcHPWL(const LunaCore::QPlacer::PlacerNetlist &netlist);
};

namespace LunaCore::QLAPlacer
{

    bool place(const ChipDB::Rect64 &regionRect, const ChipDB::Module *mod);

};

