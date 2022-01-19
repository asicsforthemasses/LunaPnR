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
    MyObject(const std::string &name) : m_name(name) {}

    const std::string& name() const
    {
        return m_name;
    }

protected:
    std::string m_name;
};

BOOST_AUTO_TEST_CASE(check_NamedStorage)
{
    std::cout << "--== CHECK NAMEDSTORAGE ==--\n";

    ChipDB::NamedStorage<MyObject> storage;

    // at() does not throw!
    BOOST_CHECK_NO_THROW(storage.at(0));
    BOOST_CHECK(storage.at(0) == nullptr);

    auto obj1 = std::make_shared<MyObject>("Obj #1");
    auto obj2 = std::make_shared<MyObject>("Obj #2");

    storage.add(obj1);
    storage.add(obj2);

    BOOST_CHECK(storage.at("Obj #1").ptr() == obj1);
    BOOST_CHECK(storage.at("Obj #2").ptr() == obj2);

    BOOST_CHECK(storage.remove("Obj #1"));
    BOOST_CHECK(!storage.at("Obj #1").isValid());
    BOOST_CHECK(storage.at("Obj #2").ptr() == obj2);

    BOOST_CHECK(storage.remove("Obj #2"));
    BOOST_CHECK(!storage.at("Obj #1").isValid());
    BOOST_CHECK(!storage.at("Obj #2").isValid());

    BOOST_CHECK(storage.remove("FakeName") == false);
}

struct MyListener : public ChipDB::INamedStorageListener
{
    MyListener() : 
        m_mostRecentKey(ChipDB::ObjectNotFound),
        m_mostRecentNotificationType(NotificationType::UNSPECIFIED)
    {
    }

    void notify(ChipDB::ObjectKey key, NotificationType t) override
    {
        m_mostRecentKey = key;
        m_mostRecentNotificationType = t;
    }

    ChipDB::ObjectKey m_mostRecentKey;
    NotificationType m_mostRecentNotificationType;
};

BOOST_AUTO_TEST_CASE(check_Notifier)
{
    std::cout << "--== CHECK NAMEDSTORAGE NOTIFIER ==--\n";

    MyListener listener;
    ChipDB::NamedStorage<MyObject> storage;
    storage.addListener(&listener);

    BOOST_CHECK(listener.m_mostRecentKey == -1);
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::UNSPECIFIED);

    auto objKey = storage.add(std::make_shared<MyObject>("Obj1"));

    BOOST_CHECK(listener.m_mostRecentKey == objKey.value().key());
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::ADD);

    auto objKey2 = storage.add(std::make_shared<MyObject>("Obj1"));

    BOOST_CHECK(listener.m_mostRecentKey == objKey2.value().key()); 
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::ADD);

    BOOST_CHECK(storage.remove("Obj1"));

    BOOST_CHECK(listener.m_mostRecentKey == objKey.value().key());
    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::REMOVE);

    storage.clear();

    BOOST_CHECK(listener.m_mostRecentNotificationType == ChipDB::INamedStorageListener::NotificationType::CLEARALL);
}

BOOST_AUTO_TEST_SUITE_END()