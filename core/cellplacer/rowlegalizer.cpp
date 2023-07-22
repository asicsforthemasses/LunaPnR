// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "rowlegalizer.h"
#include "common/logging.h"

#include <list> 
#include <cmath>

using namespace LunaCore;

static ChipDB::CoordType roundToNearestValidPosition(ChipDB::CoordType pos, const ChipDB::CoordType minCellWidth)
{
    ChipDB::CoordType v = (pos + (minCellWidth/2)) / minCellWidth;
    return v * minCellWidth;
}

static ChipDB::CoordType roundToLowestValidPosition(ChipDB::CoordType pos, const ChipDB::CoordType minCellWidth)
{
    ChipDB::CoordType v = pos/ minCellWidth;
    return v * minCellWidth;
}

void Legalizer::Cluster::addCell(const ChipDB::CoordType cellXPos, const Cell &cell, CellIndex cellIdx)
{
    m_lastCellIndex = cellIdx;
    m_totalWeight   += cell.m_weight;
    m_q += cell.m_weight*(static_cast<double>(cellXPos) - m_totalWidth);
    m_totalWidth += cell.m_size.m_x;
}

void Legalizer::Cluster::addCluster(Cluster &cluster)
{
    m_lastCellIndex = cluster.m_lastCellIndex;
    m_totalWeight += cluster.m_totalWeight;
    m_q += cluster.m_q - cluster.m_totalWeight*m_totalWidth;
    m_totalWidth += cluster.m_totalWidth;
}

void Collapse(const Legalizer::Row &row, std::list<Legalizer::Cluster> &clusters, 
    std::list<Legalizer::Cluster>::iterator clusterIter, 
    const ChipDB::CoordType minCellWidth)
{
    auto &cluster = *clusterIter;
    auto const optPos = cluster.optimalPosition();
    auto xc = row.m_rect.left() + 
        roundToNearestValidPosition(optPos - row.m_rect.left(), minCellWidth);

    const auto xmin = row.m_rect.left();
    const auto xmax = xmin + row.m_rect.width();

    xc = std::max(xmin, xc);
    xc = std::min(xmax - cluster.m_totalWidth, xc);

    if (clusterIter != clusters.begin())
    {
        auto prevClusterIter = std::prev(clusterIter);

        auto &cluster2 = *prevClusterIter;  // previous cluster
        auto xc2 = row.m_rect.left() +
            roundToNearestValidPosition(cluster2.optimalPosition() - row.m_rect.left(), minCellWidth);

        if ((xc2 + cluster2.m_totalWidth) > xc)
        {
            // merge cluster 1 into cluster 2
            cluster2.addCluster(cluster);
            auto iter = clusters.erase(clusterIter);
            Collapse(row, clusters, --iter, minCellWidth);
        }
    }
}

void Legalizer::placeRow(std::vector<Cell> &cells, const Row &row, const ChipDB::CoordType minCellWidth)
{
    std::list<Cluster> clusters;

    bool firstCell = true;
    for(CellIndex cellIdx = 0; cellIdx < row.m_cellIdxs.size(); cellIdx++)
    {
        auto &cell = cells.at(row.m_cellIdxs.at(cellIdx));

        // Well, here's a problem. We want to place the cell on a legal grid position
        // in order to do that we round the x coordinate to the nearest available
        // position. But we might round it to fall just outside the position of the 
        // row. This is undesirable.
        //
        // Some cells are positioned outside the row, whether we round to the
        // nearest valid position or not. These will generate an infinte cost.
        // This is by design.
        // 
        // In an attempt to fix this situation, we try regular rounding first.
        // If the left cell edge is outside the row, we round xpos down to the lower
        // grid position and try again.

        auto cellXPos = row.m_rect.left() +
            roundToNearestValidPosition(cell.m_globalPos.m_x - row.m_rect.left(), minCellWidth);

        // Is the cell outside the row? if so, round down.
        if (cellXPos >= row.m_rect.right())
        {
            cellXPos = row.m_rect.left() +
                roundToLowestValidPosition(cell.m_globalPos.m_x - row.m_rect.left(), minCellWidth);
        }

        if (firstCell)
        {
            auto &cluster = clusters.emplace_back();

            cluster.init();
            cluster.m_xleft = cellXPos;
            cluster.m_firstCellIndex = cellIdx;
            
            cluster.addCell(cellXPos, cell, cellIdx);
            firstCell = false;
        }
        else
        {
            auto &cluster = clusters.back();
            const auto clusterRightEdge = cluster.m_xleft + cluster.m_totalWidth;

            if (clusterRightEdge <= cellXPos)
            {
                // create a new cluster
                clusters.emplace_back();

                auto &newCluster = clusters.back();
                newCluster.init();
                newCluster.m_xleft = cellXPos;
                newCluster.m_firstCellIndex = cellIdx;

                newCluster.addCell(cellXPos, cell, cellIdx);
            }
            else
            {
                // cell will overlap!
                cluster.addCell(cellXPos, cell, cellIdx);
                Collapse(row, clusters, std::prev(clusters.end()), minCellWidth);
            }
        }
    }

    // set the new positions for all the cells
    for(auto const& cluster : clusters)
    {
        auto x = cluster.m_xleft;

        x = std::max(x, row.m_rect.left());

        for(size_t idx = cluster.m_firstCellIndex; idx <= cluster.m_lastCellIndex; idx++)
        {
            auto &cell = cells.at(row.m_cellIdxs.at(idx));
            cell.m_legalPos = ChipDB::Coord64{x, row.m_rect.bottom()};

            switch(row.m_rowType)
            {
            case ChipDB::RowType::FLIPY:
                cell.m_orientation = ChipDB::Orientation::MX;
                break;
            default:
                cell.m_orientation = ChipDB::Orientation::R0;
                break;
            }
            
            x += cell.m_size.m_x;
        }
    }
}

double LunaCore::Legalizer::calcRowCost(const std::vector<Cell> &cells, const Row &row)
{
    double cost = 0;
    for(auto cellIdx : row.m_cellIdxs)
    {
        auto const& cell = cells.at(cellIdx);
        cost += cell.m_weight*std::abs(cell.m_legalPos.m_x - cell.m_globalPos.m_x);
        cost += cell.m_weight*std::abs(cell.m_legalPos.m_y - cell.m_globalPos.m_y);

        // make cost as large as possible if one of the cells
        // is outside the row.

        const auto cellRightEdge = cell.m_legalPos.m_x + cell.m_size.m_x;
        const auto cellLeftEdge  = cell.m_legalPos.m_x;

        if (cellRightEdge > row.m_rect.right())
        {
            return std::numeric_limits<double>::max();
        }
        else if (cellLeftEdge < row.m_rect.left())
        {
            return std::numeric_limits<double>::max();
        }
    }

    return cost;
}

bool LunaCore::Legalizer::legalizeRegion(const ChipDB::Region &region, ChipDB::Netlist &netlist)
{
    const auto minCellWidth  = region.getMinCellSize().m_x;

    if (minCellWidth == 0)
    {
        Logging::doLog(Logging::LogType::ERROR,"Legalizer::legalizeRegion: min cell width has not been defined in region.\n");
        return false;
    }

    // create a vector consisting of all placed but movable instances
    std::vector<Cell> cells;
    for(auto ins : netlist.m_instances)
    {
        if (!ins.isValid())
        {
            continue;
        }

        if (ins->m_placementInfo == ChipDB::PlacementInfo::PLACED)    
        {
            auto &cell = cells.emplace_back();

            cell.m_size = ins->instanceSize();
            cell.m_globalPos = ins->m_pos;
            cell.m_weight = 1.0f;   // FIXME: what should this be??!
            cell.m_instanceKey = ins.key();
        }
    }

    // sort cells according to x-position
    std::sort(cells.begin(), cells.end(), [&cells](auto const & cell1, auto const & cell2)
        {
            if (cell1.m_globalPos.m_x < cell2.m_globalPos.m_x)
            {
                return true;
            }

            return false;
        }
    );

    // create custom row legalizer Row objects that mirror the Rows in the region.
    // FIXME: insert fixed objects into the row legalizer Rows.
    //        things like fixed end-caps 
    //        we also need some kind of provision for density constraints
    //        w.r.t. decap and filler cells.

    std::vector<Row> rows;
    rows.resize(region.m_rows.size());
    
    for(size_t rowIdx=0; rowIdx < region.m_rows.size(); rowIdx++)
    {
        rows.at(rowIdx).m_rect    = region.m_rows.at(rowIdx).m_rect;
        rows.at(rowIdx).m_rowType = region.m_rows.at(rowIdx).m_rowType;
    }

    // try the sorted cells in each row and see which row has the lowest
    // placement cost
    for(CellIndex cellIdx=0; cellIdx < cells.size(); cellIdx++)
    {
        double bestCost = std::numeric_limits<double>::max();

        size_t bestRowIdx  = 0;
        size_t rowIndex = 0;
        for(auto &row : rows)
        {
            row.insertCell(cellIdx);
            placeRow(cells, row, minCellWidth);
            auto cost = calcRowCost(cells, row);
            if (cost < bestCost)
            {
                bestCost    = cost;
                bestRowIdx  = rowIndex;
            }
            row.removeLastCell();
            rowIndex++;
        }

        auto &bestRow = rows.at(bestRowIdx);
        bestRow.insertCell(cellIdx);
        placeRow(cells, bestRow, minCellWidth);
    }

    // write back the legal positions of the cells
    for(auto const& cell : cells)
    {
        assert(cell.m_instanceKey >= 0);
        netlist.m_instances.at(cell.m_instanceKey)->m_pos = cell.m_legalPos;
        netlist.m_instances.at(cell.m_instanceKey)->m_orientation = cell.m_orientation;
    }

    return true;
}

#if 0
void LunaCore::Legalizer::identifyFillerCells(const ChipDB::Design &design)
{
    auto cellLib = design.cellLib();
    for(auto const& cell : cellLib)
    {
        //if (cell.)
    }
}

void LunaCore::Legalizer::insertFillerCell(ChipDB::Netlist &netlist,
    const ChipDB::Coord64 &pos, const ChipDB::CoordType width)
{
    auto fillerPos = pos;
    std::stringstream ss;
    ss << "__filler" << m_fillerID++;

#if 0
    auto fillerInstance = std::make_shared<ChipDB::Instance>(ss.str(), ChipDB::InstanceType::CELL, cell);
    fillerInstance->m_pos = fillerPos;
    netlist.m_instances.add(fillerInstance);
#endif
}
#endif
