#pragma once
#include <list>
#include <cassert>

#include "design/design.h"

namespace LunaCore::Legalizer
{

    struct Cell
    {
        ChipDB::InstanceBase *m_ins;
        ChipDB::Coord64 m_globalPos;    ///< position before legalization
        ChipDB::Coord64 m_legalPos;     ///< position after legalization
        ChipDB::Coord64 m_size;         ///< cell size
        double          m_weight;       ///< mathematical weight of cell (number of connections?)
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
        ChipDB::Rect64 m_rect;
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

    void placeRow(std::vector<Cell> &cells, Row &row, const ChipDB::CoordType cellMinWidth);
    double calcRowCost(const std::vector<Cell> &cells, const Row &row);

    void legalizeRegion(const ChipDB::Region &region, ChipDB::Netlist &netlist, ChipDB::CoordType minCellWidth);
};