#include "lunacore.h"

#include <string>
#include <array>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(NetlistSplitterTest)

struct VerticalSelector : LunaCore::QPlacer::Selector
{
    virtual bool operator()(LunaCore::QPlacer::PlacerNodeId id,
        const LunaCore::QPlacer::PlacerNode &node)
    {
        return (node.m_pos.m_y < m_verticalCutline);
    }

    ChipDB::CoordType m_verticalCutline;
};

BOOST_AUTO_TEST_CASE(check_netlist_splitter)
{
    std::cout << "--== CHECK NETLIST SPLITTER ==--\n";

    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

    // use the analytical placer to 
    // place a string of cells
    // and split the netlist
    // based on their vertical position

    LunaCore::QPlacer::Placer placer;
    LunaCore::QPlacer::PlacerNetlist netlist;

    auto &nodes = netlist.m_nodes;
    auto &nets  = netlist.m_nets;

    // create netlist
    nets.emplace_back();
    nets.at(0).m_weight = 1;
    nets.at(0).m_nodes.push_back(0);
    nets.at(0).m_nodes.push_back(1);

    nets.emplace_back();
    nets.at(1).m_weight = 1;
    nets.at(1).m_nodes.push_back(1);
    nets.at(1).m_nodes.push_back(2);

    nets.emplace_back();
    nets.at(2).m_weight = 1;
    nets.at(2).m_nodes.push_back(2);
    nets.at(2).m_nodes.push_back(3);

    nodes.emplace_back();
    nodes.at(0).m_connections.push_back(0);
    nodes.at(0).m_pos = {0,0};
    nodes.at(0).m_type = LunaCore::QPlacer::PlacerNodeType::FixedNode;

    nodes.emplace_back();
    nodes.at(1).m_connections.push_back(0);
    nodes.at(1).m_connections.push_back(1);
    nodes.at(1).m_pos = {0,0};
    nodes.at(1).m_type = LunaCore::QPlacer::PlacerNodeType::MovableNode;

    nodes.emplace_back();
    nodes.at(2).m_connections.push_back(1);
    nodes.at(2).m_connections.push_back(2);
    nodes.at(2).m_pos = {0,0};
    nodes.at(2).m_type = LunaCore::QPlacer::PlacerNodeType::MovableNode;

    nodes.emplace_back();
    nodes.at(3).m_connections.push_back(2);
    nodes.at(3).m_pos = {100,300};
    nodes.at(3).m_type = LunaCore::QPlacer::PlacerNodeType::FixedNode;

    Eigen::VectorXd xpos;
    Eigen::VectorXd ypos;

    placer.solve(netlist, xpos, ypos);

    // cell should be a linear string from 0.0 to 100,300
    std::array<ChipDB::Coord64,4> checkPos =
    {
        ChipDB::Coord64{0,0},
        ChipDB::Coord64{33,100},
        ChipDB::Coord64{66,200},
        ChipDB::Coord64{100,300}
    };
    
    // check locations of movable nodes
    for(size_t idx = 0; idx < checkPos.size(); idx++)
    {
        if (!nodes.at(idx).isFixed())
        {
            float error = std::abs(xpos[idx] - checkPos.at(idx).m_x) + 
                std::abs(ypos[idx] - checkPos.at(idx).m_y);

            BOOST_CHECK(error < 1.0f);

            netlist.m_nodes.at(idx).m_pos = ChipDB::Coord64{
                static_cast<ChipDB::CoordType>(xpos[idx]), 
                static_cast<ChipDB::CoordType>(ypos[idx])};  
        }

        auto const& node = netlist.m_nodes.at(idx);

        doLog(LOG_VERBOSE,"  Node %d placed at %lu,%lu\n",idx, node.m_pos.m_x, node.m_pos.m_y);
    }

    // split the netlist based on the vertical position
    VerticalSelector selector;
    selector.m_verticalCutline = 250;
    auto splitNetlist = LunaCore::QPlacer::createNetlistFromSelection(
        netlist, selector
    );

    doLog(LOG_VERBOSE, "Netlist after selection:\n");
    splitNetlist.dump(std::cout);

    BOOST_CHECK(splitNetlist.numberOfNets() == 2);
    BOOST_CHECK(splitNetlist.numberOfNodes() == 3);

    selector.m_verticalCutline = 150;
    splitNetlist = LunaCore::QPlacer::createNetlistFromSelection(
        netlist, selector
    );

    BOOST_CHECK(splitNetlist.numberOfNets() == 1);
    BOOST_CHECK(splitNetlist.numberOfNodes() == 2);

    setLogLevel(ll);
}

BOOST_AUTO_TEST_SUITE_END()