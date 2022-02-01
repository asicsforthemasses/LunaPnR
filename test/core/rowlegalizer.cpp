/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#include "lunacore.h"

#include <string>
#include <array>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(RowLegalizerTest)

BOOST_AUTO_TEST_CASE(check_legal_positions)
{
    auto netlist = std::make_shared<ChipDB::Netlist>();

    auto mycell = std::make_shared<ChipDB::Cell>("myCell");
    mycell->m_size = ChipDB::Coord64{800, 10000};

    auto ins1 = std::make_shared<ChipDB::Instance>("ins1", mycell);
    auto ins2 = std::make_shared<ChipDB::Instance>("ins2", mycell);
    auto ins3 = std::make_shared<ChipDB::Instance>("ins3", mycell);
    auto ins4 = std::make_shared<ChipDB::Instance>("ins4", mycell);

    netlist->m_instances.add(ins1);
    netlist->m_instances.add(ins2);
    netlist->m_instances.add(ins3);
    netlist->m_instances.add(ins4);

    std::vector<LunaCore::Legalizer::Cell> cells;
    // cell 1 at 10000, 8000    
    cells.emplace_back();
    cells.back().m_instanceKey  = netlist->m_instances.at("ins1").key();
    cells.back().m_globalPos    = ChipDB::Coord64{10000, 8000};
    cells.back().m_size         = mycell->m_size;
    cells.back().m_weight       = 1;

    // cell 2 abutted to cell 1 on x-axis
    cells.emplace_back();
    cells.back().m_instanceKey  = netlist->m_instances.at("ins2").key();
    cells.back().m_globalPos    = ChipDB::Coord64{10800, 4000};
    cells.back().m_size         = mycell->m_size;
    cells.back().m_weight       = 1;

    // cell 3 overlaps cell 2 on x-axis
    cells.emplace_back();
    cells.back().m_instanceKey  = netlist->m_instances.at("ins3").key();
    cells.back().m_globalPos = ChipDB::Coord64{10900, 4000};
    cells.back().m_size         = mycell->m_size;
    cells.back().m_weight = 1;

    // cell 4 far away from cell 1,2 and 3
    cells.emplace_back();
    cells.back().m_instanceKey  = netlist->m_instances.at("ins4").key();
    cells.back().m_globalPos = ChipDB::Coord64{20000, 8000};
    cells.back().m_size         = mycell->m_size;
    cells.back().m_weight = 1;

    LunaCore::Legalizer::Row row;
    row.m_rect = ChipDB::Rect64{{5000, 0},{30000, 10000}};

    row.m_cellIdxs.push_back(0);
    row.m_cellIdxs.push_back(1);
    row.m_cellIdxs.push_back(2);
    row.m_cellIdxs.push_back(3);
    LunaCore::Legalizer::placeRow(cells, row, 800);

    ssize_t idx = 0;
    for(auto const& cell : cells)
    {
        std::stringstream ss;
        ss << "Cell " << idx << "  start pos: " << cell.m_globalPos << "  legal pos: " << cell.m_legalPos << "\n";
        doLog(LOG_INFO, ss);
        idx++;
    }

    auto cost = calcRowCost(cells, row);
    doLog(LOG_INFO, "Placement cost: %lf\n", cost);

    BOOST_TEST(cost == 25100.0);
    BOOST_CHECK((cells.at(0).m_legalPos == ChipDB::Coord64{9800,0}));
    BOOST_CHECK((cells.at(1).m_legalPos == ChipDB::Coord64{10600,0}));
    BOOST_CHECK((cells.at(2).m_legalPos == ChipDB::Coord64{11400,0}));
    BOOST_CHECK((cells.at(3).m_legalPos == ChipDB::Coord64{20200,0}));
}

BOOST_AUTO_TEST_SUITE_END()
