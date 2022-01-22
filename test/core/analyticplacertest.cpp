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

BOOST_AUTO_TEST_SUITE(AnalyticPlacerTest)

BOOST_AUTO_TEST_CASE(check_analytic_placer)
{
    std::cout << "--== CHECK ANALYTIC PLACER ==--\n";

    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

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
    nodes.at(0).setLLPos({0,0});
    nodes.at(0).m_type = LunaCore::QPlacer::PlacerNodeType::FixedNode;

    nodes.emplace_back();
    nodes.at(1).m_connections.push_back(0);
    nodes.at(1).m_connections.push_back(1);
    nodes.at(1).setLLPos({0,0});
    nodes.at(1).m_type = LunaCore::QPlacer::PlacerNodeType::MovableNode;

    nodes.emplace_back();
    nodes.at(2).m_connections.push_back(1);
    nodes.at(2).m_connections.push_back(2);
    nodes.at(2).setLLPos({0,0});
    nodes.at(2).m_type = LunaCore::QPlacer::PlacerNodeType::MovableNode;

    nodes.emplace_back();
    nodes.at(3).m_connections.push_back(2);
    nodes.at(3).setLLPos({100,300});
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
        doLog(LOG_VERBOSE,"  Node %d placed at %.2f,%.2f\n",idx, xpos[idx], ypos[idx]);

        if (!nodes.at(idx).isFixed())
        {
            float error = std::abs(xpos[idx] - checkPos.at(idx).m_x) + 
                std::abs(ypos[idx] - checkPos.at(idx).m_y);

            BOOST_CHECK(error < 1.0f);
        }
    }

    setLogLevel(ll);
}

BOOST_AUTO_TEST_CASE(place_multiplier)
{
    std::cout << "--== ANALYTIC PLACER: place multiplier ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::ifstream verilogfile("test/files/verilog/multiplier.v");
    BOOST_CHECK(verilogfile.good());

    ChipDB::Verilog::Reader::load(design, verilogfile);

    auto modKeyObjPair = design.m_moduleLib.lookupModule("multiplier");

    BOOST_CHECK(modKeyObjPair.isValid());
    BOOST_CHECK(modKeyObjPair->m_netlist);

    // allocate pin instances
    int64_t left_y  = 0;
    int64_t right_y = 0;
    for(auto insKeyObjPair : modKeyObjPair->m_netlist->m_instances)
    {
        if (insKeyObjPair->m_insType == ChipDB::InstanceType::PIN)
        {
            auto const& pin = insKeyObjPair->getPin(0);
            if (pin.m_pinInfo->isInput())
            {
                insKeyObjPair->m_pos = {0, left_y};
                insKeyObjPair->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
                left_y += 5000;
            }
            else
            {
                insKeyObjPair->m_pos = {65000, right_y};
                insKeyObjPair->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;
                right_y += 5000;
            }
        }
    }

    // create a floorplan with region
    auto region = std::make_shared<ChipDB::Region>("core");
    region->m_site = "corehd";
    region->m_rect.setSize( {65000+20000,65000+20000} );
    region->m_halo = ChipDB::Margins64{10000,10000,10000,10000};

    design.m_floorplan.m_regions.add(region);

    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

    //FIXME: 
#if 0  
    BOOST_CHECK(LunaCore::QPlacer::placeModuleInRegion(&design, mod, region));
    // check locations of movable nodes
    for(size_t idx = 0; idx < mod.; idx++)
    {
        if (!nodes.at(idx).isFixed())
        {
            doLog(LOG_VERBOSE,"  Node %d placed at %.2f,%.2f\n",idx, xpos[idx], ypos[idx]);
        }
    }
    
#endif

    setLogLevel(ll);
}

BOOST_AUTO_TEST_SUITE_END()