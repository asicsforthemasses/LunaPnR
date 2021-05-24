#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(NamedStorage)

class MyObject
{
public:
    std::string m_name;
};

BOOST_AUTO_TEST_CASE(check_Coord64)
{
    std::cout << "--== CHECK NAMEDSTORAGE ==--\n";

    ChipDB::NamedStorage<MyObject*> storage;

    auto obj1 = new MyObject();
    obj1->m_name = "Obj #1";

    auto obj2 = new MyObject();
    obj2->m_name = "Obj #2";

    storage.add(obj1->m_name, obj1);
    storage.add(obj2->m_name, obj2);

    BOOST_CHECK(storage.lookup("Obj #1") == obj1);
    BOOST_CHECK(storage.lookup("Obj #2") == obj2);

    BOOST_CHECK(storage.remove("Obj #1"));
    BOOST_CHECK(storage.lookup("Obj #1") == nullptr);
    BOOST_CHECK(storage.lookup("Obj #2") == obj2);    

    BOOST_CHECK(storage.remove("Obj #2"));
    BOOST_CHECK(storage.lookup("Obj #1") == nullptr);
    BOOST_CHECK(storage.lookup("Obj #2") == nullptr);

    BOOST_CHECK(storage.remove("FakeName") == false);
}


BOOST_AUTO_TEST_SUITE_END()