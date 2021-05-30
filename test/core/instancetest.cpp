#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(InstanceTest)

BOOST_AUTO_TEST_CASE(various_instance_tests)
{
    std::cout << "--== INSTANCE TEST ==--\n";

    ChipDB::Cell cell;
    cell.m_name = "henk";
    cell.createPin("A");

    ChipDB::Instance ins(&cell);
    ins.m_name = "frans";

    BOOST_CHECK(ins.cell() != nullptr);
    BOOST_CHECK(ins.getArchetypeName() == "henk");
    BOOST_CHECK(ins.getPinIndex("dummy") < 0);  // check non-existing pin returns < 0
    BOOST_CHECK(ins.getPinIndex("A") >= 0);     // check existing pin returns >= 0
    BOOST_CHECK(!ins.isModule());

    // check unconnected pin returns nullptr as net
    auto pinIndex = ins.getPinIndex("A");
    BOOST_CHECK(ins.getConnectedNet(pinIndex) == nullptr);
    
    BOOST_CHECK(ins.getConnectedNet(100) == nullptr);   // check non-existing pin returns nullptr
    
    // check pin iterator
    cell.createPin("B");
    cell.createPin("Z");

    size_t pinCount = 0;
    for(auto const& pinInfo : ins.pinInfos())
    {
        pinCount++;
    }
    BOOST_CHECK(pinCount == 3);

    BOOST_CHECK(ins.getNumberOfPins() == 3);

    // check instance-is-module stuff
    ChipDB::Module mod;
    mod.m_name = "MyModule";
    mod.createPin("Z");

    ChipDB::Instance ins2(&mod);
    ins2.m_name = "diederik";
    BOOST_CHECK(ins2.isModule());
}

BOOST_AUTO_TEST_SUITE_END()
