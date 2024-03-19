// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "padring.hpp"
#include "common/matrix.h"
#include "common/logging.h"

namespace LunaCore::Padring
{

void Padring::clear()
{
    m_top.clear();
    m_bottom.clear();
    m_left.clear();
    m_right.clear();

    m_lowerLeftCorner.clear();
    m_lowerRightCorner.clear();
    m_upperLeftCorner.clear();
    m_upperRightCorner.clear();

    m_lowerLeftCorner.m_itemType = LayoutItem::ItemType::CORNER;
    m_lowerRightCorner.m_itemType = LayoutItem::ItemType::CORNER;
    m_upperLeftCorner.m_itemType = LayoutItem::ItemType::CORNER;
    m_upperRightCorner.m_itemType = LayoutItem::ItemType::CORNER;
};

/*
    The placement algorithm uses 1D quadratic cost minimization.
    Coordinates are normalized w.r.t the smallest filler cell
    and final coordinates are integers.
    This way, gaps between pads can always be filled.

    Force acted upon a cell 'x' with width 'wx',
    and 'wa' is the width of the cell 'a' to the left.
    and b is the cell to the right.

    F1 = k*(x-a-wa), F2 = k*(b-x-wx)

    Set k=1:
    F1 = x-a-wa, F2 = b-x-wx

    d/dx F1^2 + F2^2 = -2(a+b+wa-wb-2x)

    which can be normalized to (a+b+wa-wb-2x)

    The standard form is:
    Ax - b = 0

    Or equivalently:
    Ax = b

    with a and b corner cells, they are fixed
    and do not appear in the vector 'x'.

    (a+b+wa-wx-2x) = 0
    a and b are fixed, and so are wa and wx
    so we can rewrite it as:

    -2x = wx-wa-a-b

    for 'a' and 'b' fixed:
        A += -2
        b += wx-wa-a-b

    for 'a' fixed, 'b' moveable:
        A(x) += -2
        A(b) += -1
        b(x) = wx-wa-a

    for 'a' and 'b' moveable:
        A(x) += -2
        A(a) += -1
        A(b) += -1
        b(x) = wx-wa
*/
bool Padring::layout(Database &db)
{
    // place corners
    auto dieSize = db.m_design.m_floorplan->dieSize();
    m_upperLeftCorner.m_pos  = 0;
    m_upperRightCorner.m_pos = dieSize.m_x - m_upperRightCorner.m_size;

    if (m_upperRightCorner.m_pos < m_upperLeftCorner.m_size)
    {
        Logging::logError("Not enough die space to fit the corner cells");
        return false;
    }

    auto availableWidth = dieSize.m_x - m_upperLeftCorner.m_size - m_lowerLeftCorner.m_size;

    std::vector<LayoutItem> items;
    items.reserve(m_top.cellCount() + 2);   // pads and two corners
    items.push_back(m_upperLeftCorner);

    ChipDB::CoordType totalPadWidth = 0;
    for(auto const& item : m_top)
    {
        if (item->m_itemType == LayoutItem::ItemType::CELL)
        {
            items.push_back(*item.get());
            totalPadWidth += item->m_size;
        }
    }

    items.push_back(m_upperRightCorner);

    if (availableWidth < totalPadWidth)
    {
        Logging::logError("Not enough die space to fit the pads");
        Logging::logError("  Total pad width: %ld\n", totalPadWidth);
        Logging::logError("  Available width: %ld\n", availableWidth);
        return false;
    }

    // layout north

    int matrixDimension = m_top.cellCount();
    LunaCore::Matrix A;
    A.reserveRows(matrixDimension);

    Eigen::VectorXd Bvec(matrixDimension);
    Bvec.setZero();

    auto iter = std::next(items.begin());   // points to first moveable pad/cell
    auto last = std::prev(items.end());     // points to last corner cell

    std::size_t rowIdx = 0;
    while(iter != last)
    {
        auto const& prevItem = *std::prev(iter);
        auto const& nextItem = *std::next(iter);

        if ((prevItem.m_itemType == LayoutItem::ItemType::CELL) &&
            (nextItem.m_itemType == LayoutItem::ItemType::CELL))
        {
            // both prev and next items moveable
            A(rowIdx, rowIdx) += 2.0;
            A(rowIdx, rowIdx-1) += -1.0;
            A(rowIdx, rowIdx+1) += -1.0;
            Bvec(rowIdx) = prevItem.m_size - iter->m_size;
        }
        else if (prevItem.m_itemType == LayoutItem::ItemType::CELL)
        {
            // prev item moveable
            A(rowIdx, rowIdx) += 2.0;
            A(rowIdx, rowIdx-1) += -1.0;
            Bvec(rowIdx) = prevItem.m_size - iter->m_size + nextItem.m_pos;
        }
        else if (nextItem.m_itemType == LayoutItem::ItemType::CELL)
        {
            // next item moveable
            A(rowIdx, rowIdx) += 2.0;
            A(rowIdx, rowIdx+1) += -1.0;
            Bvec(rowIdx) = prevItem.m_size - iter->m_size + prevItem.m_pos;
        }
        else
        {
            // no moveable items
        }

        rowIdx++;
        iter++;
    }

    Eigen::ConjugateGradient<Eigen::SparseMatrix<double>, Eigen::Upper | Eigen::Lower> solver;
    Eigen::SparseMatrix<double> eigenAmat(matrixDimension, matrixDimension);
    toEigen(A, eigenAmat);

    eigenAmat.makeCompressed();
    solver.compute(eigenAmat);

    Eigen::VectorXd Xvec;
    Xvec = solver.solve(Bvec);

    if (Logging::getLogLevel() == Logging::LogType::VERBOSE)
    {
        std::stringstream ss;
        ss << "\n A = \n";
        ss << Eigen::MatrixXd(eigenAmat) << "\n";

        Logging::logVerbose("%s\n", ss.str().c_str());

        ss.str("");
        ss << "\n B = \n";
        ss << Bvec << "\n";

        Logging::logVerbose("%s\n", ss.str().c_str());

        Logging::logVerbose("Padring placement result:\n");
        std::size_t idx = 0;
        for(auto const v : Xvec)
        {
            Logging::logVerbose("  %ld %f\n", idx++, v);
        }
    }

    // fix the position of the cells
    rowIdx = 0;
    for(const auto &item : items)
    {
        if (item.m_itemType == LayoutItem::ItemType::CELL)
        {
            auto modPtr = db.m_design.getTopModule();
            auto insKp = modPtr->m_netlist->lookupInstance(item.m_instanceName);
            if (!insKp.isValid())
            {
                Logging::logError("Cannot find instance %s in netlist\n", item.m_instanceName.c_str());
                return false;
            }

            insKp->m_pos.m_x = static_cast<ChipDB::CoordType>(std::round(Bvec(rowIdx)));
            insKp->m_pos.m_y = dieSize.m_y - insKp->instanceSize().m_y;
            insKp->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
            rowIdx++;
        }
    }

    return true;
}

};