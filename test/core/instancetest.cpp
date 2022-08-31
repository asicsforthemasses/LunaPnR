// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

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

    auto cell = std::make_shared<ChipDB::Cell>("henk");
    cell->createPin("A");
    cell->createPin("B");
    cell->createPin("Z");

    auto insPtr = std::make_shared<ChipDB::Instance>("frans", ChipDB::InstanceType::CELL, cell);

    BOOST_CHECK(insPtr);
    BOOST_CHECK(insPtr->cell() != nullptr);
    BOOST_CHECK(insPtr->getArchetypeName() == "henk");
    BOOST_CHECK(!insPtr->getPin("dummy").isValid());   // check non-existing pin returns false
    BOOST_CHECK(insPtr->getPin("A").isValid());        // check existing pin returns true
    BOOST_CHECK(!insPtr->isModule());

    // check unconnected pin returns ObjectNotFound
    auto pin = insPtr->getPin("A");
    BOOST_CHECK(pin.netKey() == ChipDB::ObjectNotFound);
        
    size_t pinCount = 0;
    for(auto pinKey=0; pinKey < insPtr->getNumberOfPins(); pinKey++)
    {
        if (insPtr->getPin(pinKey).isValid())
        {
            pinCount++;
        }
    }
    
    BOOST_CHECK(pinCount == 3);
    BOOST_CHECK(insPtr->getNumberOfPins() == 3);

    // check net connections
    // they should all be unconnected
    pinCount = 0;
    for(auto connectedNet : insPtr->connections())
    {
        BOOST_CHECK(connectedNet == -1);
        pinCount++;
    }

    std::cout << "pincount " << pinCount << "\n";
    BOOST_CHECK(pinCount == 3);

    // check instance-is-module stuff
    auto modPtr = std::make_shared<ChipDB::Module>("MyModule");
    modPtr->createPin("Z");

    auto insPtr2 = std::make_shared<ChipDB::Instance>("diederik", ChipDB::InstanceType::MODULE, modPtr);
    BOOST_CHECK(insPtr2->isModule());

    // Regression: test that the net key is correct when looking a pin up by name
    BOOST_CHECK(insPtr->setPinNet(0 /* pin A */, 123));
    BOOST_CHECK(insPtr->getPin("A").netKey() == 123);   // lookup by name
    BOOST_CHECK(insPtr->getPin(0).netKey() == 123);   // lookup by id for good measure

    BOOST_CHECK(insPtr->setPinNet(1 /* pin B */, 456));
    BOOST_CHECK(insPtr->getPin("B").netKey() == 456);   // lookup by name
    BOOST_CHECK(insPtr->getPin(1).netKey() == 456);   // lookup by id for good measure

    // test pin A gain to see if we haven't overwritten anything
    BOOST_CHECK(insPtr->getPin("A").netKey() == 123);   // lookup by name
    BOOST_CHECK(insPtr->getPin(0).netKey() == 123);   // lookup by id for good measure    
}

BOOST_AUTO_TEST_SUITE_END()
