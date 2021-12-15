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

BOOST_AUTO_TEST_CASE(check_NamedStorage)
{
    std::cout << "--== CHECK NAMEDSTORAGE ==--\n";

    ChipDB::NamedStorage<MyObject*, true> storage;

    // at() does not throw!
    BOOST_CHECK_NO_THROW(storage.at(0));
    BOOST_CHECK(storage.at(0) == nullptr);

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

struct MyListener : public ChipDB::INamedStorageListener
{
    MyListener() : 
        m_mostRecentIndex(-1),
        m_mostRecentNotificationType(NotificationType::UNSPECIFIED)
    {
    }

    void notify(int32_t userID, ssize_t index, NotificationType t) override
    {
        m_mostRecentIndex = index;
        m_mostRecentNotificationType = t;
    }

    ssize_t m_mostRecentIndex;
    NotificationType m_mostRecentNotificationType;
};

BOOST_AUTO_TEST_CASE(check_Notifier)
{
    std::cout << "--== CHECK NAMEDSTORAGE NOTIFIER ==--\n";

    MyListener listener;
    ChipDB::NamedStorage<MyObject*, true> storage;
    storage.addListener(&listener);

    BOOST_CHECK(listener.m_mostRecentIndex == -1);
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::UNSPECIFIED);

    storage.add("Obj1", new MyObject());

    BOOST_CHECK(listener.m_mostRecentIndex == 0); 
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::ADD);

    storage.add("Obj2", new MyObject());

    BOOST_CHECK(listener.m_mostRecentIndex == 1); 
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::ADD);

    BOOST_CHECK(storage.remove("Obj1"));

    BOOST_CHECK(listener.m_mostRecentIndex == 0); 
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::REMOVE);

    storage.clear();

    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::REMOVE);
}

BOOST_AUTO_TEST_SUITE_END()