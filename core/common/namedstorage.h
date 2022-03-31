/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/


#pragma once

#include <type_traits>
#include <sstream>
#include <vector>
#include <string>
#include <atomic>
#include <algorithm>
#include <memory>
#include <unordered_map>
#include <iterator>

#include "dbtypes.h"

#ifdef NO_SSIZE_T
#include <type_traits>
typedef std::make_signed<size_t>::type ssize_t;
#endif

namespace ChipDB {

/** return type for object creation function so we can return both
*/
template<class T> struct KeyObjPair
{
    constexpr KeyObjPair() = default;
    constexpr KeyObjPair(ObjectKey key, std::shared_ptr<T> objPtr) : m_key(key), m_objPtr(objPtr) {}

    constexpr std::shared_ptr<T> operator->() noexcept
    {
        return m_objPtr;
    }

    constexpr const std::shared_ptr<T> operator->() const noexcept
    {
        return m_objPtr;
    }

    constexpr T& operator*() noexcept
    {
        return *m_objPtr;
    }

    constexpr T& operator*() const noexcept
    {
        return *m_objPtr;
    }

    constexpr ObjectKey key() const noexcept
    {
        return m_key;
    }

    constexpr std::shared_ptr<T> ptr() const noexcept
    {
        return m_objPtr;
    }

    constexpr T* rawPtr() const noexcept
    {
        return m_objPtr.get();
    }

    constexpr bool isValid() const noexcept
    {
        return (m_key >= 0) && m_objPtr;
    }

    friend bool operator==(const KeyObjPair<T> &lhs, const KeyObjPair<T> &rhs) noexcept
    {
        return (lhs.m_objPtr == rhs.m_objPtr) && (lhs.m_key == rhs.m_key);
    }

    friend bool operator!=(const KeyObjPair<T> &lhs, const KeyObjPair<T> &rhs) noexcept
    {
        return (lhs.m_objPtr != rhs.m_objPtr) || (lhs.m_key != rhs.m_key);
    }

protected:
    std::shared_ptr<T> m_objPtr = nullptr;
    ObjectKey          m_key = ObjectNotFound;
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

/** container to store object pointers and provides fast named lookup. 
 * 
 *  TODO: when updating to C++20, use concepts to contrain the type to INamedObject
 * 
*/

template <class T>
class NamedStorage
{
public:

    using ContainerType = std::unordered_map<ObjectKey, std::shared_ptr<T> >;

    NamedStorage() = default;

    virtual ~NamedStorage()
    {
    }
    
    void clear()
    {
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

    /** adds an named object to the container. 
     *  returns ObjectAlreadyExists if an object with the same name already exists. 
     *  returns the ObjectKey is the object was successfully added. */
    std::optional<KeyObjPair<T> > add(std::shared_ptr<T> objectPtr)
    {
        auto iter = m_nameToKey.find(objectPtr->name());
        if (iter == m_nameToKey.end())
        {
            // no such named object, okay to add!
            auto key = generateUniqueObjectKey();
            m_objects[key] = objectPtr;
            m_nameToKey[objectPtr->name()] = key;
            notifyAll(key, INamedStorageListener::NotificationType::ADD);
            return KeyObjPair(key, objectPtr);
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
                m_objects.erase(objIter);
                notifyAll(key, INamedStorageListener::NotificationType::REMOVE);
            }
            return true;
        }
        return false;
    }

    /** remove an object by key. returns true if successful */
    bool remove(ObjectKey key)
    {        
        auto iter = m_objects.find(key);
        if (iter == m_objects.end())
        {
            return false;   // no such object
        }
        else
        {
            auto obj = iter->second;
            auto nameToKeyIter = m_nameToKey.find(obj->name());

            if (nameToKeyIter != m_nameToKey.end())
            {
                m_nameToKey.erase(nameToKeyIter);
            }

            m_objects.erase(iter);

            notifyAll(key, INamedStorageListener::NotificationType::REMOVE);
            return true;
        }
        return false;
    }

    /** access an object by name. Will throw std::out_of_range exception when the object does not exist */
    KeyObjPair<T> at(const std::string &name)
    {        
        auto objKey = m_nameToKey.at(name);
        return KeyObjPair<T>(objKey, m_objects.at(objKey));
    }

    /** access an object by name. Will throw std::out_of_range exception when the object does not exist */
    KeyObjPair<T> at(const std::string &name) const
    {
        auto objKey = m_nameToKey.at(name);
        return m_objects.at(objKey);
    }

    /** access an object by key. Will throw std::out_of_range exception when the object does not exist */
    constexpr std::shared_ptr<T> at(ObjectKey key)
    {
        return m_objects.at(key);
    }
 
    /** access an object by key. Will throw std::out_of_range exception when the object does not exist */
    constexpr const std::shared_ptr<T> at(ObjectKey key) const
    {
        return m_objects.at(key);
    }

    /** access an object by key. Will return a nullptr when the object does not exist */
    constexpr std::shared_ptr<T> operator[](ObjectKey key)
    {
        return findObject(key);
    }
 
    /** access an object by key. Will return a nullptr when the object does not exist */
    constexpr const std::shared_ptr<T> operator[](ObjectKey key) const
    {
        return findObject(key);
    }

    /** access an object by name. Will return a nullptr when the object does not exist */
    constexpr KeyObjPair<T> operator[](const std::string &name)
    {
        return findObject(name);
    }
 
    /** access an object by name. Will return a nullptr when the object does not exist */
    constexpr KeyObjPair<T> operator[](const std::string &name) const
    {
        return findObject(name);
    }

    class Iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = KeyObjPair<T>;

        using BaseIteratorType  = typename ContainerType::iterator;

        Iterator() = default;
        Iterator(const Iterator &) = default;

        Iterator(BaseIteratorType baseIterator) 
            : m_baseIterator(baseIterator) {}

        constexpr value_type operator*()
        {
            return KeyObjPair(m_baseIterator->first, m_baseIterator->second);
        }

        constexpr value_type operator->()
        {
            return KeyObjPair(m_baseIterator->first, m_baseIterator->second);
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

    class ConstIterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = KeyObjPair<T>;

        using BaseIteratorType  = typename ContainerType::const_iterator;

        ConstIterator() = default;
        ConstIterator(const ConstIterator &) = default;
        
        ConstIterator(BaseIteratorType baseIterator) 
            : m_baseIterator(baseIterator) {}

        constexpr value_type operator*()
        {
            return KeyObjPair(m_baseIterator->first, m_baseIterator->second);
        }

        constexpr value_type operator->()
        {
            return KeyObjPair(m_baseIterator->first, m_baseIterator->second);
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
            Iterator tmp = *this;
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

    Iterator begin()
    {
        return Iterator(m_objects.begin());
    }

    ConstIterator begin() const
    {
        return ConstIterator(m_objects.begin());
    }

    Iterator end()
    {
        return Iterator(m_objects.end());
    }

    ConstIterator end() const
    {
        return ConstIterator(m_objects.end());
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

    using iterator = Iterator;
    using const_iterator = ConstIterator;

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

    ObjectKey generateUniqueObjectKey() const
    {
        return m_uniqueObjectKey++;
    }

    std::shared_ptr<T> findObject(ObjectKey key)
    {
        auto objIter = m_objects.find(key);
        if (objIter == m_objects.end())
        {
            return nullptr;
        }
        return objIter->second;
    }

    const std::shared_ptr<T> findObject(ObjectKey key) const
    {
        auto objIter = m_objects.find(key);
        if (objIter == m_objects.end())
        {
            return nullptr;
        }
        return objIter->second;
    }

    KeyObjPair<T> findObject(const std::string &name)
    {
        auto objKeyIter = m_nameToKey.find(name);
        if (objKeyIter == m_nameToKey.end())
        {
            return KeyObjPair<T>();
        }

        auto objIter = m_objects.find(objKeyIter->second);
        if (objIter == m_objects.end())
        {
            return KeyObjPair<T>();
        }        

        return KeyObjPair<T>(objIter->first, objIter->second);
    }

    KeyObjPair<T> findObject(const std::string &name) const
    {
        auto objKeyIter = m_nameToKey.find(name);
        if (objKeyIter == m_nameToKey.end())
        {
            return KeyObjPair<T>();
        }

        auto objIter = m_objects.find(objKeyIter->second);
        if (objIter == m_objects.end())
        {
            return KeyObjPair<T>();
        }        

        return KeyObjPair<T>(objIter->first, objIter->second);
    }

    mutable ObjectKey m_uniqueObjectKey = 0;
    std::vector<ListenerData> m_listeners;
    ContainerType m_objects;
    std::unordered_map<std::string, ObjectKey> m_nameToKey;
};





};

