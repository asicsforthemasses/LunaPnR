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
    auto const dieSize = db.m_design.m_floorplan->dieSize();

    // place the corner cells
    placeInstance(db, m_upperLeftCorner.m_instanceName,
        ChipDB::Coord64{0, dieSize.m_y - m_upperLeftCorner.m_height},
        ChipDB::Orientation{ChipDB::Orientation::R0});

    placeInstance(db, m_upperRightCorner.m_instanceName,
        ChipDB::Coord64{dieSize.m_y - m_upperRightCorner.m_width, dieSize.m_y - m_upperRightCorner.m_height},
        ChipDB::Orientation{ChipDB::Orientation::R90});

    placeInstance(db, m_lowerLeftCorner.m_instanceName,
        ChipDB::Coord64{0,0},
        ChipDB::Orientation{ChipDB::Orientation::R180});

    placeInstance(db, m_lowerRightCorner.m_instanceName,
        ChipDB::Coord64{dieSize.m_y - m_lowerRightCorner.m_width,0},
        ChipDB::Orientation{ChipDB::Orientation::R270});

    ChipDB::Rect64 rect;

    rect.setLeft(0);
    rect.setRight(dieSize.m_x);
    rect.setTop(dieSize.m_y);
    rect.setBottom(dieSize.m_y - db.m_design.m_floorplan->ioMargins().m_top);
    m_top.setLayoutRect(rect);
    m_top.setCellOrientation(ChipDB::Orientation{ChipDB::Orientation::R180});
    Logging::logVerbose("Top rect   : (%ld %ld) (%ld %ld)\n",
        rect.left(), rect.bottom(),
        rect.right(), rect.top());
    m_top.setDirection(Layout::Direction::HORIZONTAL);

    rect.setLeft(0);
    rect.setRight(dieSize.m_x);
    rect.setTop(db.m_design.m_floorplan->ioMargins().m_bottom);
    rect.setBottom(0);
    m_bottom.setLayoutRect(rect);
    m_bottom.setCellOrientation(ChipDB::Orientation{ChipDB::Orientation::R0});
    Logging::logVerbose("Bottom rect: (%ld %ld) (%ld %ld)\n",
        rect.left(), rect.bottom(),
        rect.right(), rect.top());
    m_bottom.setDirection(Layout::Direction::HORIZONTAL);

    rect.setLeft(0);
    rect.setRight(db.m_design.m_floorplan->ioMargins().m_left);
    rect.setTop(dieSize.m_y);
    rect.setBottom(0);
    m_left.setLayoutRect(rect);
    m_left.setCellOrientation(ChipDB::Orientation{ChipDB::Orientation::R90});
    Logging::logVerbose("Left rect  : (%ld %ld) (%ld %ld)\n",
        rect.left(), rect.bottom(),
        rect.right(), rect.top());
    m_left.setDirection(Layout::Direction::VERTICAL);

    rect.setLeft(dieSize.m_x - db.m_design.m_floorplan->ioMargins().m_right);
    rect.setRight(dieSize.m_x);
    rect.setTop(dieSize.m_y);
    rect.setBottom(0);
    m_right.setLayoutRect(rect);
    m_right.setCellOrientation(ChipDB::Orientation{ChipDB::Orientation::R270});
    Logging::logVerbose("Right rect : (%ld %ld) (%ld %ld)\n",
        rect.left(), rect.bottom(),
        rect.right(), rect.top());
    m_right.setDirection(Layout::Direction::VERTICAL);

    layoutEdge(db, m_upperLeftCorner, m_upperRightCorner, m_top);
    layoutEdge(db, m_lowerLeftCorner, m_lowerRightCorner, m_bottom);
    layoutEdge(db, m_lowerLeftCorner, m_upperLeftCorner, m_left);
    layoutEdge(db, m_lowerRightCorner, m_upperRightCorner, m_right);

    return true;
}

bool Padring::layoutEdge(Database &db, const LayoutItem &corner1, const LayoutItem &corner2, const Layout &edge)
{
    auto const dieSize = db.m_design.m_floorplan->dieSize();

    ChipDB::CoordType corner1Pos = 0;
    ChipDB::CoordType corner2Pos = 0;
    ChipDB::CoordType availableWidth = 0;

    if (edge.direction() == Layout::Direction::HORIZONTAL)
    {
        corner2Pos = dieSize.m_x - corner2.m_width;
        availableWidth = dieSize.m_x - corner1.m_width - corner2.m_width;
    }
    else
    {
        corner2Pos = dieSize.m_y - corner2.m_width;
        availableWidth = dieSize.m_y - corner1.m_width - corner2.m_width;
    }

    if (availableWidth < 0)
    {
        Logging::logError("Not enough die space to fit the corner cells");
        return false;
    }

    std::vector<LayoutItem> items;
    items.reserve(edge.cellCount() + 2);   // pads and two corners
    items.push_back(corner1);
    items.back().m_pos = corner1Pos;

    ChipDB::CoordType totalPadWidth = 0;
    for(auto const& item : edge)
    {
        if (item->m_itemType == LayoutItem::ItemType::CELL)
        {
            items.push_back(*item.get());
            totalPadWidth += item->m_width;
        }
    }

    items.push_back(corner2);
    items.back().m_pos = corner2Pos;

    if (availableWidth < totalPadWidth)
    {
        Logging::logError("Not enough die space to fit the pads");
        Logging::logError("  Total pad width: %ld\n", totalPadWidth);
        Logging::logError("  Available width: %ld\n", availableWidth);
        return false;
    }

    // layout edge
    int matrixDimension = edge.cellCount();
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
            Bvec(rowIdx) = prevItem.m_width - iter->m_width;
        }
        else if (prevItem.m_itemType == LayoutItem::ItemType::CELL)
        {
            // prev item moveable
            A(rowIdx, rowIdx) += 2.0;
            A(rowIdx, rowIdx-1) += -1.0;
            Bvec(rowIdx) = prevItem.m_width - iter->m_width + nextItem.m_pos;
        }
        else if (nextItem.m_itemType == LayoutItem::ItemType::CELL)
        {
            // next item moveable
            A(rowIdx, rowIdx) += 2.0;
            A(rowIdx, rowIdx+1) += -1.0;
            Bvec(rowIdx) = prevItem.m_width - iter->m_width + prevItem.m_pos;
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
            ChipDB::Coord64 pos;

            if (edge.direction() == Layout::Direction::HORIZONTAL)
            {
                pos.m_x = static_cast<ChipDB::CoordType>(std::round(Xvec(rowIdx)));
                pos.m_y = edge.layoutRect().bottom();
            }
            else
            {
                pos.m_y = static_cast<ChipDB::CoordType>(std::round(Xvec(rowIdx)));
                pos.m_x = edge.layoutRect().left();
            }

            if (!placeInstance(db, item.m_instanceName, pos, edge.cellOrientation()))
            {
                Logging::logError("Cannot find instance %s in netlist\n", item.m_instanceName.c_str());
                return false;
            }

            rowIdx++;
        }
    }
    return true;
}

bool Padring::placeInstance(Database &db,
    const std::string &insName,
    const ChipDB::Coord64 &lowerLeftPos,
    const ChipDB::Orientation &orientation)
{
    auto modPtr = db.m_design.getTopModule();
    auto insKp = modPtr->m_netlist->lookupInstance(insName);

    if (!insKp.isValid())
    {
        Logging::logError("Cannot find instance %s in netlist\n", insName);
        return false;
    }

    insKp->m_pos = lowerLeftPos;
    insKp->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
    insKp->m_orientation = orientation;
    return true;
}

};
