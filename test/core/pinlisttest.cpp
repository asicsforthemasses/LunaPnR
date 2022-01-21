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