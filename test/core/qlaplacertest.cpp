#include "lunacore.h"

#include <string>
#include <array>
#include <vector>
#include <memory>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(QLAPlacerTest)

bool createStringOfInstancesConnectingTwoTerminals(ChipDB::Design &design, ChipDB::Module *mod)
{
    // create two terminals
    auto *srcTerminalCell = design.m_cellLib.createCell("SrcTerminal");
    auto outPin = srcTerminalCell->m_pins.createPin("out");
    outPin->m_iotype = ChipDB::IOType::OUTPUT;

    auto *dstTerminalCell = design.m_cellLib.createCell("DstTerminal");
    auto inPin = dstTerminalCell->m_pins.createPin("in");
    inPin->m_iotype = ChipDB::IOType::INPUT;

    auto *insCell   = design.m_cellLib.createCell("InsCell");
    auto inInsPin   = insCell->m_pins.createPin("in");
    auto outInsPin  = insCell->m_pins.createPin("out");
    
    inInsPin->m_iotype  = ChipDB::IOType::INPUT;
    outInsPin->m_iotype = ChipDB::IOType::OUTPUT;

    auto *ins = new ChipDB::Instance(srcTerminalCell);
    ins->m_name = "src";
    if (!mod->addInstance(ins))
    {
        return false;
    }

    ins = new ChipDB::Instance(dstTerminalCell);
    ins->m_name = "dst";
    if (!mod->addInstance(ins))
    {
        return false;
    }

    for(size_t i=0; i<5; i++)
    {
        std::stringstream ss;
        ss << "cell" << i;
        auto *ins = new ChipDB::Instance(insCell);
        ins->m_name = ss.str();
        if (!mod->addInstance(ins))
        {
            return false;
        }

        auto *pin1 = ins->getPinInfo("in");
        auto *pin2 = ins->getPinInfo("out");

        if (pin1 == nullptr)
        {
            return false;
        }

        if (pin2 == nullptr)
        {
            return false;
        }        

        if (pin1->m_iotype != ChipDB::IOType::INPUT)
        {
            return false;
        }

        if (pin2->m_iotype != ChipDB::IOType::OUTPUT)
        {
            return false;
        }        
    }

    std::vector<ChipDB::Net*> nets;

    nets.push_back(mod->createNet("n1"));
    nets.push_back(mod->createNet("n2"));
    nets.push_back(mod->createNet("n3"));
    nets.push_back(mod->createNet("n4"));
    nets.push_back(mod->createNet("n5"));
    nets.push_back(mod->createNet("n6"));

    if (!mod->m_netlist->connect("src", "out", "n1"))
    {
        return false;
    }

    for(size_t i=0; i<5; i++)
    {
        std::stringstream ss;
        ss << "cell" << i;

        if (!mod->m_netlist->connect(ss.str(), "in", nets[i]->m_name))
        {
            return false;
        }

        if (!mod->m_netlist->connect(ss.str(), "out", nets[i+1]->m_name))
        {
            return false;
        }
    }

    if (!mod->m_netlist->connect("dst", "in", "n6"))
    {
        return false;
    }    

    return true;
}


BOOST_AUTO_TEST_CASE(check_qla_netlist_generation)
{
    std::cout << "--== CHECK QLAPLACER NETLIST GEN ==--\n";

    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

    // create a ChipDB::Netlist of two terminals and
    // a string of 5 instances that connected the terminals.

    ChipDB::Design design;
    auto mod = design.m_moduleLib.createModule("glamodule");
    BOOST_CHECK(mod != nullptr);
    mod->m_netlist.reset(new ChipDB::Netlist());
    BOOST_CHECK(mod->m_netlist);    // check netlist pointer is valid

    BOOST_CHECK(createStringOfInstancesConnectingTwoTerminals(design, mod));

    BOOST_CHECK(mod->m_netlist->m_instances.size() == 7 /* 2 terminals and 5 instances */);
    BOOST_CHECK(mod->m_netlist->m_nets.size() == 6);

    #if 1
    std::ofstream ofile("test/files/results/qlanetlist.dot");
    auto status = LunaCore::Dot::Writer::write(ofile, mod);
    BOOST_CHECK(status);
    #endif

    // create the qlanetlist
    auto qlanetlist = LunaCore::QLAPlacer::Private::createPlacerNetlist(*mod->m_netlist.get());

    BOOST_CHECK(qlanetlist.m_nets.size() == mod->m_netlist->m_nets.size());
    BOOST_CHECK(qlanetlist.m_nodes.size() == mod->m_netlist->m_instances.size());

    setLogLevel(ll);
}

struct NetlistCallback
{
    void operator()(const LunaCore::QPlacer::PlacerNetlist &netlist)
    {
        std::ofstream ofile("test/files/results/qplaplacer_placement.svg");
        LunaCore::QLAPlacer::Private::writeNetlistToSVG(ofile, m_regionRect, netlist);
    }

    ChipDB::Rect64 m_regionRect;
};

BOOST_AUTO_TEST_CASE(check_qla_netlist_placement)
{
    std::cout << "--== CHECK QLAPLACER PLACEMENT ==--\n";

    auto ll = getLogLevel();
    setLogLevel(LOG_VERBOSE);

    // create a ChipDB::Netlist of two terminals and
    // a string of 5 instances that connected the terminals.

    ChipDB::Design design;
    auto mod = design.m_moduleLib.createModule("glamodule");
    BOOST_CHECK(mod != nullptr);
    mod->m_netlist.reset(new ChipDB::Netlist());
    BOOST_CHECK(mod->m_netlist);    // check netlist pointer is valid

    BOOST_CHECK(createStringOfInstancesConnectingTwoTerminals(design, mod));

    // fix src pin at the top (1000,1000)
    auto srcPin = mod->m_netlist->m_instances.lookup("src");
    BOOST_CHECK(srcPin != nullptr);
    srcPin->m_pos = ChipDB::Coord64{1000,1000};
    srcPin->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;

    // fix dst pin at the bottom (1000,0)
    auto dstPin = mod->m_netlist->m_instances.lookup("dst");
    BOOST_CHECK(dstPin != nullptr);
    dstPin->m_pos = ChipDB::Coord64{1000,0};
    dstPin->m_placementInfo = ChipDB::PlacementInfo::PLACEDANDFIXED;

    ChipDB::Region region;
    region.m_rect = ChipDB::Rect64{{0,0}, {2000,1000}};

    NetlistCallback callback;
    callback.m_regionRect = region.m_rect;

    auto status = LunaCore::QLAPlacer::place(region, *(mod->m_netlist.get()), callback);
    BOOST_CHECK(status);

    // dump qlanetlist
    status = LunaCore::NetlistTools::writePlacementFile(std::cout, mod->m_netlist.get());
    BOOST_CHECK(status);

    // check that all movable nodes are inside the rectangle
    for(auto ins : mod->m_netlist->m_instances)
    {
        auto placed = (ins->m_placementInfo == ChipDB::PlacementInfo::PLACEDANDFIXED) || 
            (ins->m_placementInfo == ChipDB::PlacementInfo::PLACED);

        BOOST_CHECK(placed);
    }


    setLogLevel(ll);
}

BOOST_AUTO_TEST_SUITE_END()