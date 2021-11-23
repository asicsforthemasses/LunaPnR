#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PinInfoListTest)

BOOST_AUTO_TEST_CASE(check_pininfolist)
{
    std::cout << "--== CHECK PININFOLIST ==--\n";
    
    ChipDB::PinInfoList list;

    // check non-existing pin lookup returns nullptr
    BOOST_CHECK(list.lookup("alfred") == nullptr);
    
    // check index of non-existing pin is -1
    BOOST_CHECK(list.lookupIndex("alfred") == -1);

    // create pin 'henk'
    auto p1 = list.createPin("henk");
    BOOST_CHECK(p1->m_name == "henk");

    // check index of henk is 0
    BOOST_CHECK(list.lookupIndex("henk") == 0);
    
    // check pin 'henk' is not a nullptr
    auto henkPtr = list.lookup("henk");
    BOOST_CHECK(henkPtr != nullptr);

    // check createPin on henk returns the original henk
    BOOST_CHECK(list.createPin("henk") == henkPtr);

    // check that the size of the pinlist is 1
    BOOST_CHECK(list.size() == 1);
};


BOOST_AUTO_TEST_SUITE_END()