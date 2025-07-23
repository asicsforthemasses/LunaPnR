// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <list>
#include <vector>
#include <cassert>

#include "database/database.h"

namespace LunaCore
{

class Legalizer
{
public:

    struct Cell
    {
        ChipDB::ObjectKey   m_instanceKey;  ///< key to the instance in the netlist
        ChipDB::Coord64     m_globalPos;    ///< position before legalization
        ChipDB::Coord64     m_legalPos;     ///< position after legalization
        ChipDB::Coord64     m_size;         ///< cell size
        ChipDB::Orientation m_orientation;  ///< cell orientation
        double              m_weight;       ///< mathematical weight of cell (number of connections?)
    };

    using CellIndex = ssize_t;

    struct Cluster
    {
        double      m_totalWeight;
        size_t      m_firstCellIndex;
        size_t      m_lastCellIndex;
        ChipDB::CoordType   m_totalWidth;
        ChipDB::CoordType   m_xleft;
        double      m_q;

        constexpr void init()
        {
            m_totalWeight = 0.0;
            m_firstCellIndex = 0;
            m_lastCellIndex  = 0;
            m_totalWidth = 0;
            m_q = 0.0;
        }

        constexpr ChipDB::CoordType optimalPosition() const noexcept
        {
            assert(m_totalWeight > 0.0);
            return m_q / m_totalWeight;
        }

        void addCell(const ChipDB::CoordType cellPos, const Cell &cell, CellIndex cellIdx);
        void addCluster(Cluster &cluster);
    };

    struct Row
    {
        ChipDB::RowType m_rowType;
        ChipDB::Rect64  m_rect;
        std::vector<CellIndex> m_cellIdxs;

        void insertCell(CellIndex idx)
        {
            m_cellIdxs.push_back(idx);
        }

        void removeLastCell()
        {
            m_cellIdxs.pop_back();
        }
    };

    [[nodiscard]] bool legalize(
        const ChipDB::Floorplan &floorplan,
        ChipDB::Netlist &netlist);

protected:

    /** layout all the cells in a row and update the cell vector accordingly */
    void placeRow(std::vector<Cell> &cells, const Row &row, const ChipDB::CoordType cellMinWidth);

    double calcRowCost(const std::vector<Cell> &cells, const Row &row);
};

};
