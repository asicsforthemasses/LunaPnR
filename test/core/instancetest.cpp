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

    auto insPtr = std::make_shared<ChipDB::Instance>("frans", cell);

    BOOST_CHECK(insPtr);
    BOOST_CHECK(insPtr->cell() != nullptr);
    BOOST_CHECK(insPtr->getArchetypeName() == "henk");
    BOOST_CHECK(!insPtr->getPin("dummy").isValid());   // check non-existing pin returns false
    BOOST_CHECK(insPtr->getPin("A").isValid());        // check existing pin returns true
    BOOST_CHECK(!insPtr->isModule());

    // check unconnected pin returns ObjectNotFound
    auto const& pin = insPtr->getPin("A");
    BOOST_CHECK(pin.m_netKey == ChipDB::ObjectNotFound);
        
    // check pin iterator
    cell->createPin("B");
    cell->createPin("Z");

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

    // check instance-is-module stuff
    auto modPtr = std::make_shared<ChipDB::Module>("MyModule");
    modPtr->createPin("Z");

    auto insPtr2 = std::make_shared<ChipDB::Instance>("diederik", modPtr);
    BOOST_CHECK(insPtr2->isModule());
}

BOOST_AUTO_TEST_SUITE_END()
