// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PinInfoListTest)


BOOST_AUTO_TEST_CASE(check_pin)
{
    std::cout << "--== CHECK PIN ==--\n";

    ChipDB::Instance::Pin pin;

    // check defaults
    BOOST_CHECK(!pin.isValid());
    BOOST_CHECK(pin.netKey() == ChipDB::ObjectNotFound);
    BOOST_CHECK(pin.pinKey() == ChipDB::ObjectNotFound);
    BOOST_CHECK(pin.name() == "INVALID PININFO");

    // create a valid pin
    pin.m_netKey = 123;
    pin.m_pinKey = 456;
    pin.m_pinInfo = std::make_shared<ChipDB::PinInfo>();
    BOOST_CHECK(pin.isValid());

    // check that we can copy a pin
    auto pin2 = pin;
    BOOST_CHECK(pin.isValid());
    BOOST_CHECK(pin.netKey() == 123);
    BOOST_CHECK(pin.pinKey() == 456);
}

BOOST_AUTO_TEST_CASE(check_pininfolist)
{
    ChipDB::KeyObjPair<float> t;
    std::cout << "--== CHECK PININFOLIST ==--\n";
    
    ChipDB::PinInfoList list;

    // check non-existing pin lookup returns nullptr
    BOOST_CHECK(!list["alfred"].isValid());
    
    // create pin 'henk'
    auto p1 = list.createPin("henk");
    BOOST_CHECK(p1->name() == "henk");

    // check index of henk is 0
    BOOST_CHECK(list["henk"].key() == 0);
    
    // check pin 'henk' is not a nullptr
    auto henkKeyObjPair = list["henk"];
    BOOST_CHECK(henkKeyObjPair.isValid());

    // check createPin on henk returns the original henk
    BOOST_CHECK(list.createPin("henk") == henkKeyObjPair);

    // check that the size of the pinlist is 1
    BOOST_CHECK(list.size() == 1);
};


BOOST_AUTO_TEST_SUITE_END()