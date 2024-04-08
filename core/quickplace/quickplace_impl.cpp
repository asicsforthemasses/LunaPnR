// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "common/logging.h"
#include "quickplace_impl.hpp"

namespace LunaCore::QuickPlace
{

/*
    For two moveable cells:
        d/df = 2f - 2g  A(f,f) = 2,     A(f,g) = -2
        d/dg = 2g - 2f  A(g,g) = 2,     A(g,f) = -2

    for moveable cell f, fixed g:
        d/df = 2f - 2g  A(f,f) = 2, b(f) = 2*g
*/

void PlacerImpl::addConnectionToSystem(
    ChipDB::InstanceObjectKey ins1Key,
    const ChipDB::Instance& ins1,
    ChipDB::InstanceObjectKey ins2Key,
    const ChipDB::Instance& ins2,
    float weight) noexcept
{
    const float padWeight = 1.0f;   ///FIXME: should be algorithm parameter

    if (!ins1.isFixed() && !ins2.isFixed())
    {
        auto const row1 = m_insKey2row.at(ins1Key);
        auto const row2 = m_insKey2row.at(ins2Key);

        m_A.at(row1,row1) += weight;
        m_A.at(row1,row2) -= weight;

        m_A.at(row2,row2) += weight;
        m_A.at(row2,row1) -= weight;
    }
    else if (!ins1.isFixed())
    {
        // only ins1 is moveable
        auto const row1 = m_insKey2row.at(ins1Key);
        m_A.at(row1,row1) += weight * padWeight;
        m_Bx.at(row1) += weight * ins2.m_pos.m_x * padWeight;
        m_By.at(row1) += weight * ins2.m_pos.m_y * padWeight;
    }
    else if (!ins2.isFixed())
    {
        // only ins2 is moveable
        auto const row2 = m_insKey2row.at(ins2Key);
        m_A.at(row2,row2) += weight * padWeight;

        m_Bx.at(row2) += weight * ins1.m_pos.m_x * padWeight;
        m_By.at(row2) += weight * ins1.m_pos.m_y * padWeight;
    }
}



void PlacerImpl::addConnectionToSystem(
    RowIndex anchorRow,
    ChipDB::InstanceObjectKey ins2Key,
    const ChipDB::Instance& ins2,
    float weight) noexcept
{
    const float padWeight = 1.0f;   ///FIXME: should be algorithm parameter

    if (!ins2.isFixed())
    {
        auto const row2 = m_insKey2row.at(ins2Key);
        m_A.at(anchorRow,anchorRow) += weight;
        m_A.at(anchorRow,row2) -= weight;

        m_A.at(row2,row2) += weight;
        m_A.at(row2,anchorRow) -= weight;
    }
    else
    {
        // only anchor is moveable
        m_A.at(anchorRow,anchorRow) += weight;

        if (m_Bx.size() <= anchorRow) m_Bx.resize(anchorRow+1);
        if (m_By.size() <= anchorRow) m_By.resize(anchorRow+1);

        m_Bx.at(anchorRow) += weight * ins2.m_pos.m_x * padWeight;
        m_By.at(anchorRow) += weight * ins2.m_pos.m_y * padWeight;
    }
}

bool PlacerImpl::place(Database &db, ChipDB::Module &mod)
{
    auto netlistPtr = mod.m_netlist;
    if (!netlistPtr)
    {
        Logging::logError("Quickplace: cannot place module %s - it doesn't have a netlist\n",
            mod.name().c_str());
        return false;
    }

    //FIXME: make sure all the top-level pin instances
    // have been fixed.

    auto &instances = netlistPtr->m_instances;
    auto &nets = netlistPtr->m_nets;

    // assign an instance to a row in the A matrix
    std::size_t rowIdx = 0;
    for(auto insKp : instances)
    {
        if (!insKp.isValid()) continue;   // is this an error? -> probably

        if (!insKp->isFixed())
        {
            m_insKey2row[insKp.key()] = rowIdx++;
        }
    }

    // initialize the vectors and matrix to the correct size
    auto const rowCount = rowIdx;

    m_A.resize(rowCount);
    m_Bx.resize(rowCount);
    m_By.resize(rowCount);

    // FIXME: are these really necessary?
    m_Bx.zero();
    m_By.zero();

    std::size_t anchorCount = 0;
    std::size_t skipNets = 0;
    for(auto netKp : nets)
    {
        auto const instancesOnNet = netKp->numberOfConnections();
        if (instancesOnNet < 2) continue;   // skip degenerate nets

        if (instancesOnNet == 2)
        {
            float w = 1.0f/static_cast<float>(instancesOnNet-1); // net weight

            auto iter1 = netKp->begin();
            auto iter2 = std::next(iter1);

            assert(iter1 != netKp->end());
            assert(iter2 != netKp->end());

            auto ins1Ptr = instances.at(iter1->m_instanceKey);
            auto ins2Ptr = instances.at(iter2->m_instanceKey);

            if ((!ins1Ptr) || (!ins2Ptr))
            {
                Logging::logError("Quickplace: an instance in netlist is nullptr\n");
                return false;
            }

            addConnectionToSystem(iter1->m_instanceKey, *ins1Ptr, iter2->m_instanceKey, *ins2Ptr, w);
        }
        else if (instancesOnNet == 3)
        {
            float w = 1.0f/static_cast<float>(instancesOnNet-1); // net weight

            auto iter1 = netKp->begin();
            auto iter2 = std::next(iter1);
            auto iter3 = std::next(iter2);

            assert(iter1 != netKp->end());
            assert(iter2 != netKp->end());
            assert(iter3 != netKp->end());

            auto ins1Ptr = instances.at(iter1->m_instanceKey);
            auto ins2Ptr = instances.at(iter2->m_instanceKey);
            auto ins3Ptr = instances.at(iter3->m_instanceKey);

            if ((!ins1Ptr) || (!ins2Ptr) || (!ins3Ptr))
            {
                Logging::logError("Quickplace: an instance in netlist is nullptr\n");
                return false;
            }

            addConnectionToSystem(iter1->m_instanceKey, *ins1Ptr, iter2->m_instanceKey, *ins2Ptr, w);
            addConnectionToSystem(iter2->m_instanceKey, *ins2Ptr, iter3->m_instanceKey, *ins3Ptr, w);
            addConnectionToSystem(iter3->m_instanceKey, *ins3Ptr, iter1->m_instanceKey, *ins1Ptr, w);
        }
        else
        {
            // 4+ instance nets need to be converted into a star net
            const std::size_t maxNextSize = 30; // FIXME: this should be configurable

            float w = 1.0f/static_cast<float>(instancesOnNet-1); // net weight, +1 because we have an
            if (instancesOnNet > maxNextSize)
            {
                skipNets++;
            }
            else
            {
                auto anchorRow = rowCount + anchorCount;

                for(auto const &connection : *netKp)
                {
                    auto ins2Ptr = instances.at(connection.m_instanceKey);
                    if (!ins2Ptr)
                    {
                        Logging::logError("Quickplace: an instance in netlist is nullptr\n");
                        return false;
                    }

                    addConnectionToSystem(anchorRow, connection.m_instanceKey, *ins2Ptr, w);
                }
                anchorCount++;
            }
        }
    }

    // make additional room in B for the anchor points
    // the added B elements are initialized to zero
    m_Bx.resize(m_A.rowCount());
    m_By.resize(m_A.rowCount());

    Logging::logDebug("  A matrix row count: %lu\n", m_A.rowCount());
    Logging::logDebug("  Skipped nets      : %lu\n", skipNets);
    Logging::logDebug("  Anchors           : %lu\n", anchorCount);

    LunaCore::Algebra::Vector<float> Rx(m_A.rowCount());
    LunaCore::Algebra::Vector<float> Ry(m_A.rowCount());

    // solve the linear systems
    LunaCore::Algebra::CGSolver::Preconditioner precon(m_A);

    const float maxError = 1.0e-2f; // FIXME: should this be configurable?
    auto solveInfoX = LunaCore::Algebra::CGSolver::solve(m_A, m_Bx, Rx, precon, maxError);
    auto solveInfoY = LunaCore::Algebra::CGSolver::solve(m_A, m_By, Ry, precon, maxError);

    Logging::logDebug("  Solve info X: %lu iterations\t(error: %f)\n", solveInfoX.m_iterations, solveInfoX.m_error);
    Logging::logDebug("  Solve info Y: %lu iterations\t(error: %f)\n", solveInfoY.m_iterations, solveInfoY.m_error);

    // write back the positions
    // and update cells to PLACED

    for(auto [insKey, rowIdx] : m_insKey2row)
    {
        auto insPtr = instances.at(insKey);

        if (!insPtr)
        {
            Logging::logError("Quickplace: an instance in netlist is nullptr\n");
            return false;
        }

        if (insPtr->isFixed()) continue;    // this should never happen, but sanity check.

        // position the center of the instances ..
        auto sz = insPtr->instanceSize();

        insPtr->m_pos.m_x = static_cast<ChipDB::CoordType>(Rx.at(rowIdx)) - sz.m_x/2;
        insPtr->m_pos.m_y = static_cast<ChipDB::CoordType>(Ry.at(rowIdx)) - sz.m_y/2;
        insPtr->m_placementInfo = ChipDB::PlacementInfo::PLACED;
    }

    return true;
}

}
