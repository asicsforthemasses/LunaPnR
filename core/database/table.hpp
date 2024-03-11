// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <type_traits>
#include <cassert>
#include <sstream>
#include <string>
#include <algorithm>
#include <utility>
#include <unordered_map>
#include <iterator>
#include <optional>

namespace Database
{

using ObjectKey = int32_t;
static const int32_t ObjectNotFound = -1;
static const int32_t ObjectUnspecified = -2;

/** return type for object creation function so we can return both
*/
template<typename ObjectType, typename KeyType> struct ObjectAndKey
{
    constexpr ObjectAndKey(ObjectType *obj, KeyType key) : m_key(key), m_obj(obj)
    {
        assert(obj != nullptr);
    }

    constexpr ObjectType* operator->() noexcept
    {
        return m_obj;
    }

    constexpr const ObjectType* operator->() const noexcept
    {
        return m_obj;
    }

    constexpr ObjectType& operator*() noexcept
    {
        return *m_obj;
    }

    constexpr const ObjectType& operator*() const noexcept
    {
        return *m_obj;
    }

    constexpr KeyType key() const noexcept
    {
        return m_key;
    }

    constexpr ObjectType* ptr() const noexcept
    {
        return m_obj;
    }

    friend bool operator==(const ObjectAndKey<ObjectType, KeyType> &lhs, const ObjectAndKey<ObjectType, KeyType> &rhs) noexcept
    {
        return (lhs.m_obj == rhs.m_obj) && (lhs.m_key == rhs.m_key);
    }

    friend bool operator!=(const ObjectAndKey<ObjectType, KeyType> &lhs, const ObjectAndKey<ObjectType, KeyType> &rhs) noexcept
    {
        return (lhs.m_obj != rhs.m_obj) || (lhs.m_key != rhs.m_key);
    }

protected:
    // FIXME: use NotNull
    ObjectType *m_obj{nullptr};
    KeyType     m_key = ObjectNotFound;
};


struct INamedStorageListener
{
    enum class NotificationType
    {
        UNSPECIFIED = 0,
        ADD,
        REMOVE,
        CLEARALL
    };

    /** userID: the user ID when at addListener was called to register this listener.
     *  index : the index of the item that was modified, -1 for entire collection.
     *  t     : type of modification that occurred.
    */
    virtual void notify(ObjectKey index = ObjectUnspecified, NotificationType t = NotificationType::UNSPECIFIED) = 0;
};


/** A container to create/store objects. It provides fast lookup by key and by name.
 *  Objects are owned by the table. Pointers to objects are always valid, even after
 *  inserting or removing _other_ objects.
*/

template <typename ObjectType, typename KeyType>
class Table
{
public:
    using ObjectPtr = std::add_pointer<ObjectType>::type;
    using ObjectAndKeyType = ObjectAndKey<ObjectType, KeyType>;
    using ContainerType = std::unordered_map<KeyType, ObjectPtr>;

    Table() = default;
    virtual ~Table()
    {
        clear();
    }

    // tables cannot be copied or assigned.
    Table(const Table &other) = delete;
    Table& operator=(const Table &other) = delete;

    // tables can be moved or move-assigned.
    Table(Table&& mE)            = default;
    Table& operator=(Table&& mE) = default;

    void clear()
    {
        for(auto const& pair : m_objects)
        {
            delete pair.second;
        }

        m_objects.clear();
        m_nameToKey.clear();
        m_uniqueObjectKey = 0;
        notifyAll(-1, INamedStorageListener::NotificationType::CLEARALL);
    }

    /** return the number of objects in the storage container */
    size_t size() const
    {
        return m_objects.size();
    }

    /* create an object of a specific (derived) type and insert it into the table.
       throws std::invalid_argument if the name has already been taken
       returns false if the named object already exists.

       Usage: create<MyDerivedType>("MyName", <optional argumentys> );
    */
    template<typename DerivedType, typename ... Args>
    std::optional<ObjectAndKeyType> create(const std::string &name,  Args&&... args)
    {
        if (m_nameToKey.contains(name))
        {
            std::stringstream ss;
            ss << "Table::create: name " << name << " aready taken";
            throw std::invalid_argument(ss.str());
        }

        auto obj = new DerivedType(std::forward<Args>(args)...);
        auto key = m_uniqueObjectKey++;
        m_nameToKey[name] = key;
        m_objects[key] = obj;

        obj->setName(name); // just to make sure.

        return ObjectAndKeyType{obj, key};
    }

    /*  Inserts an named object to the container. The table becomes the owner of the object
     *  throws std::invalid_argument if the object is nullptr.
     *  throws std::invalid_argument if the object name is empty.
     *  returns std::nullopt if an object with the same name already exists.
     *  returns the ObjectKey is the object was successfully inserted.
    */
    std::optional<ObjectAndKeyType> insert(ObjectType *objectPtr)
    {
        if (objectPtr == nullptr)
        {
            throw std::invalid_argument("Table::add: objectPtr cannot be null");
        }

        if (objectPtr->name().empty())
        {
            throw std::invalid_argument("Table::add: object must have a name");
        }

        auto iter = m_nameToKey.find(objectPtr->name());
        if (iter == m_nameToKey.end())
        {
            // no such named object, okay to add!
            auto key = generateUniqueObjectKey();
            m_objects[key] = objectPtr;
            m_nameToKey[objectPtr->name()] = key;
            notifyAll(key, INamedStorageListener::NotificationType::ADD);
            return ObjectAndKey(objectPtr, key);
        }

        return std::nullopt;
    }

    /** remove an object by name. returns true if successful */
    bool remove(const std::string &name)
    {
        auto iter = m_nameToKey.find(name);
        if (iter == m_nameToKey.end())
        {
            return false;   // no such named object
        }
        else
        {
            auto key = iter->second;
            m_nameToKey.erase(iter);

            auto objIter = m_objects.find(key);
            if (objIter != m_objects.end())
            {
                delete objIter->second;
                m_objects.erase(objIter);
                notifyAll(key, INamedStorageListener::NotificationType::REMOVE);
            }
            return true;
        }
        return false;
    }

    /** remove an object by key. returns true if successful */
    bool remove(KeyType key)
    {
        auto iter = m_objects.find(key);
        if (iter == m_objects.end())
        {
            return false;   // no such object
        }
        else
        {
            ObjectPtr objPtr = iter->second;
            auto nameToKeyIter = m_nameToKey.find(objPtr->name());

            if (nameToKeyIter != m_nameToKey.end())
            {
                m_nameToKey.erase(nameToKeyIter);
            }

            delete objPtr;
            m_objects.erase(iter);

            notifyAll(key, INamedStorageListener::NotificationType::REMOVE);
            return true;
        }
        return false;
    }

    /** access an object by name. Will throw std::out_of_range exception when the object does not exist */
    ObjectAndKeyType at(const std::string &name)
    {
        auto objKey = m_nameToKey.at(name);
        return ObjectAndKeyType(m_objects.at(objKey), objKey);
    }

    /** access an object by name. Will throw std::out_of_range exception when the object does not exist */
    ObjectAndKeyType at(const std::string &name) const
    {
        auto objKey = m_nameToKey.at(name);
        return ObjectAndKeyType(m_objects.at(objKey), objKey);
    }

    /** access an object by key. Will throw std::out_of_range exception when the object does not exist */
    constexpr ObjectType& at(KeyType key)
    {
        return *m_objects.at(key);
    }

    /** access an object by key. Will throw std::out_of_range exception when the object does not exist */
    constexpr const ObjectType& at(KeyType key) const
    {
        return *m_objects.at(key);
    }

    [[nodiscard]] bool contains(const std::string &name) const
    {
        return m_nameToKey.contains(name);
    }

    [[nodiscard]] bool contains(const KeyType key) const
    {
        return m_objects.contains(key);
    }

    /** returns an ObjectAndKey result if the object is found, otherwise std::nullopt */
    [[nodiscard]] std::optional<ObjectAndKeyType> find(const std::string &name) const noexcept
    {
        return findObject(name);
    }

    /** returns an ObjectAndKey result if the object is found, otherwise std::nullopt */
    [[nodiscard]] std::optional<ObjectAndKeyType> find(const KeyType key) const noexcept
    {
        ObjectPtr objPtr = findObject(key);
        if (objPtr == nullptr)
        {
            return std::nullopt;
        }

        return ObjectAndKeyType(objPtr, key);
    }

    bool rename(const std::string &oldName, const std::string &newName)
    {
        if (newName.empty()) return false;
        if (m_nameToKey.contains(newName)) return false;

        auto kp = findObject(oldName);
        if (!kp) return false;  // object not found

        m_nameToKey.erase(oldName);
        m_nameToKey[newName] = kp->key();

        kp->ptr()->setName(newName);
        return true;
    }


    template<typename ValueType>
    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ValueType;
        using reference_type    = ValueType&;

        using BaseIteratorType  = typename ContainerType::iterator;

        Iterator() = default;
        Iterator(const Iterator &) = default;

        Iterator(BaseIteratorType baseIterator)
            : m_baseIterator(baseIterator) {}

        constexpr decltype(auto) operator*()
        {
            if constexpr (std::is_same<ValueType, ObjectType>::value)
            {
                return ((const ObjectType&)*m_baseIterator->second);
            }

            if constexpr (std::is_same<ValueType, KeyType>::value)
            {
                return m_baseIterator->first;
            }

            if constexpr (std::is_same<ValueType, ObjectAndKeyType>::value)
            {
                return ObjectAndKeyType(m_baseIterator->second, m_baseIterator->first);
            }

            throw std::runtime_error("Table::ConstIterator could not deduce operator*() return type based on ValueType");
        }

        constexpr auto operator->()
        {
            if constexpr (std::is_same<ValueType, ObjectType>::value)
            {
                return m_baseIterator->second;
            }

            if constexpr (std::is_same<ValueType, KeyType>::value)
            {
                return m_baseIterator->first;
            }

            if constexpr (std::is_same<ValueType, ObjectAndKeyType>::value)
            {
                return ObjectAndKeyType(m_baseIterator->second, m_baseIterator->first);
            }

            throw std::runtime_error("Table::ConstIterator could not deduce operator*() return type based on ValueType");
        }

        // prefix increment
        Iterator& operator++()
        {
            m_baseIterator++;
            return *this;
        }

        // postfix increment
        Iterator operator++(int)
        {
            Iterator tmp = *this;
            m_baseIterator++;
            return tmp;
        }

        friend bool operator==(const Iterator &lhs, const Iterator &rhs)
        {
            return lhs.m_baseIterator == rhs.m_baseIterator;
        }

        friend bool operator!=(const Iterator &lhs, const Iterator &rhs)
        {
            return lhs.m_baseIterator != rhs.m_baseIterator;
        }

    private:
        BaseIteratorType m_baseIterator;
    };

    template<typename ValueType>
    class ConstIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = ValueType;
        using reference_type    = ValueType&;

        using BaseIteratorType  = typename ContainerType::const_iterator;

        ConstIterator() = default;
        ConstIterator(const ConstIterator &) = default;

        ConstIterator(BaseIteratorType baseIterator)
            : m_baseIterator(baseIterator) {}

        constexpr decltype(auto) operator*()
        {
            if constexpr (std::is_same<ValueType, ObjectType>::value)
            {
                return ((const ObjectType&)*m_baseIterator->second);
            }

            if constexpr (std::is_same<ValueType, KeyType>::value)
            {
                return m_baseIterator->first;
            }

            if constexpr (std::is_same<ValueType, ObjectAndKeyType>::value)
            {
                return ObjectAndKeyType(m_baseIterator->second, m_baseIterator->first);
            }

            throw std::runtime_error("Table::ConstIterator could not deduce operator*() return type based on ValueType");
        }

        constexpr auto operator->()
        {
            if constexpr (std::is_same<ValueType, ObjectType>::value)
            {
                return m_baseIterator->second;
            }

            if constexpr (std::is_same<ValueType, KeyType>::value)
            {
                return m_baseIterator->first;
            }

            if constexpr (std::is_same<ValueType, ObjectAndKeyType>::value)
            {
                return ObjectAndKeyType(m_baseIterator->second, m_baseIterator->first);
            }

            throw std::runtime_error("Table::ConstIterator could not deduce operator*() return type based on ValueType");
        }

        // prefix increment
        ConstIterator& operator++()
        {
            m_baseIterator++;
            return *this;
        }

        // postfix increment
        ConstIterator operator++(int)
        {
            ConstIterator tmp = *this;
            m_baseIterator++;
            return tmp;
        }

        friend bool operator==(const ConstIterator &lhs, const ConstIterator &rhs)
        {
            return lhs.m_baseIterator == rhs.m_baseIterator;
        }

        friend bool operator!=(const ConstIterator &lhs, const ConstIterator &rhs)
        {
            return lhs.m_baseIterator != rhs.m_baseIterator;
        }

    private:
        BaseIteratorType m_baseIterator;
    };

    auto begin()
    {
        return Iterator<ObjectType>(m_objects.begin());
    }

    auto begin() const
    {
        return ConstIterator<ObjectType>(m_objects.begin());
    }

    auto cbegin() const
    {
        return ConstIterator<ObjectType>(m_objects.begin());
    }

    auto end()
    {
        return Iterator<ObjectType>(m_objects.end());
    }

    auto end() const
    {
        return ConstIterator<ObjectType>(m_objects.end());
    }

    auto cend() const
    {
        return ConstIterator<ObjectType>(m_objects.end());
    }

    class IObjectsAndKeys
    {
    public:
        using ValueType = ObjectAndKeyType;

        IObjectsAndKeys(ContainerType &container) : m_container(container) {}

        auto begin()
        {
            return Iterator< ValueType>(m_container.begin());
        }

        auto cbegin() const
        {
            return ConstIterator<ValueType>(m_container.begin());
        }

        auto end()
        {
            return Iterator<ValueType>(m_container.end());
        }

        auto cend() const
        {
            return ConstIterator<ValueType>(m_container.end());
        }

    private:
        ContainerType &m_container;
    };

    class IConstObjectsAndKeys
    {
    public:
        using ValueType = ObjectAndKeyType;

        IConstObjectsAndKeys(const ContainerType &container) : m_container(container) {}

        auto begin() const
        {
            return ConstIterator<ObjectAndKeyType>(m_container.begin());
        }

        auto cbegin() const
        {
            return ConstIterator<ObjectAndKeyType>(m_container.begin());
        }

        auto end() const
        {
            return ConstIterator<ObjectAndKeyType>(m_container.end());
        }

        auto cend() const
        {
            return ConstIterator<ObjectAndKeyType>(m_container.end());
        }

    private:
        const ContainerType &m_container;
    };

    auto withKeys()
    {
        return IObjectsAndKeys(m_objects);
    }

    auto withKeys() const
    {
        return IConstObjectsAndKeys(m_objects);
    }

    void addListener(INamedStorageListener *listener)
    {
        // check if the listener already exists.
        auto iter = std::find_if(m_listeners.begin(), m_listeners.end(),
            [listener](auto const &listenerData)
            {
                return listenerData.m_listener == listener;
            }
        );

        // if not, add it!
        if (iter == m_listeners.end())
        {
            m_listeners.emplace_back();
            m_listeners.back().m_listener = listener;
        }
    }

    void removeListener(INamedStorageListener *listener)
    {
        auto iter = std::find_if(m_listeners.begin(), m_listeners.end(),
            [listener](auto const &listenerData)
            {
                return listenerData.m_listener == listener;
            }
        );

        if (iter != m_listeners.end())
        {
            m_listeners.erase(iter);
        }
    }

    /** trigger a generic message to all the listeners that the data has changed */
    void contentsChanged() const
    {
        notifyAll();
    }

protected:

    void notifyAll(ObjectKey key = ObjectUnspecified, INamedStorageListener::NotificationType t =
        INamedStorageListener::NotificationType::UNSPECIFIED) const
    {
        for(auto &listenerData : m_listeners)
        {
            if (listenerData.m_listener != nullptr)
            {
                listenerData.m_listener->notify(key, t);
            }
        }
    }

    struct ListenerData
    {
        INamedStorageListener *m_listener;
    };

    [[nodiscard]] constexpr KeyType generateUniqueObjectKey() const noexcept
    {
        return m_uniqueObjectKey++;
    }

    ObjectPtr findObject(ObjectKey key)
    {
        auto objIter = m_objects.find(key);
        if (objIter == m_objects.end())
        {
            return nullptr;
        }
        return objIter->second;
    }

    const ObjectPtr findObject(ObjectKey key) const
    {
        auto objIter = m_objects.find(key);
        if (objIter == m_objects.end())
        {
            return nullptr;
        }
        return objIter->second;
    }

    std::optional<ObjectAndKeyType> findObject(const std::string &name)
    {
        auto objKeyIter = m_nameToKey.find(name);
        if (objKeyIter == m_nameToKey.end())
        {
            return std::nullopt;
        }

        auto objIter = m_objects.find(objKeyIter->second);
        if (objIter == m_objects.end())
        {
            return std::nullopt;
        }

        return ObjectAndKeyType(objIter->second, objIter->first);
    }

    std::optional<ObjectAndKeyType> findObject(const std::string &name) const
    {
        auto objKeyIter = m_nameToKey.find(name);
        if (objKeyIter == m_nameToKey.end())
        {
            return std::nullopt;
        }

        auto objIter = m_objects.find(objKeyIter->second);
        if (objIter == m_objects.end())
        {
            return std::nullopt;
        }

        return ObjectAndKeyType(objIter->second, objIter->first);
    }

    mutable ObjectKey m_uniqueObjectKey{0};

    std::vector<ListenerData> m_listeners;
    ContainerType m_objects;
    std::unordered_map<std::string, KeyType> m_nameToKey;
};


};
