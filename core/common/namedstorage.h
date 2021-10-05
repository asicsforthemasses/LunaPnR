#pragma once

#include <type_traits>
#include <sstream>
#include <vector>
#include <string>
#include <atomic>
#include <algorithm>
#include <unordered_map>

namespace ChipDB {

struct INamedStorageListener
{
    enum class NotificationType
    {
        UNSPECIFIED = 0,
        ADD,
        REMOVE
    };

    virtual void notify(ssize_t index = -1, NotificationType t = NotificationType::UNSPECIFIED) = 0;
};

/** container to store object pointers and provides fast named lookup. 
*/
template <class T, bool owning = false>
class NamedStorage
{
public:
    NamedStorage()
    {
        static_assert(std::is_pointer<T>::value, "Stored object type must be a pointer");
    }

    virtual ~NamedStorage()
    {
        if constexpr (owning)
        {
            for(auto ptr : m_objects)
            {
                delete ptr;
            }
        }
    }

    void clear()
    {
        if constexpr (owning)
        {
            for(auto ptr : m_objects)
            {
                delete ptr;
            }
        }

        m_objects.clear();
        m_nameToIndex.clear();
        notifyAll();
    }

    size_t size() const
    {
        return m_objects.size();
    }

    bool add(const std::string &name, T object)
    {
        auto iter = m_nameToIndex.find(name);
        if (iter == m_nameToIndex.end())
        {
            // no such named object, okay to add!
            ssize_t index = m_objects.size()-1;
            m_objects.push_back(object);
            m_nameToIndex[name] = index;
            notifyAll(index, INamedStorageListener::NotificationType::ADD);
            return true;
        }
        return false;   // object already exists
    }

#if 0
    bool add(T object)
    {
        auto iter = m_nameToIndex.find(name);
        if (iter == m_nameToIndex.end())
        {
            // no such named object, okay to add!
            m_objects.push_back(object);
            m_nameToIndex[name] = m_objects.size()-1;
            return true;
        }
    }
#endif

    bool remove(const std::string &name)
    {
        auto iter = m_nameToIndex.find(name);
        if (iter == m_nameToIndex.end())
        {
            return false;   // no such named object
        }
        else
        {
            size_t index = iter->second;
            m_nameToIndex.erase(iter);
            if (index < m_objects.size())
            {
                m_objects[index] = nullptr;
                notifyAll(index, INamedStorageListener::NotificationType::REMOVE);
            }            
            return true;
        }
    }

    T lookup(const std::string &name)
    {
        auto iter = m_nameToIndex.find(name);
        if (iter == m_nameToIndex.end())
        {
            return nullptr;
        }
        else
        {            
            return m_objects.at(iter->second);
        }
    }

    const T lookup(const std::string &name) const
    {
        auto iter = m_nameToIndex.find(name);
        if (iter == m_nameToIndex.end())
        {
            return nullptr;
        }
        else
        {
            return m_objects.at(iter->second);
        }
    }

    T at(size_t index)
    {
        #if 0
        if (index >= m_objects.size())
        {
            std::stringstream ss;
            ss << "NamedStorage out of range: max=" << m_objects.size()-1 << " requested=" << index;
            throw std::out_of_range(ss.str());
        }
        #endif

        if (index >= m_objects.size())
        {
            return nullptr;
        }

        return m_objects.at(index);
    }
 
    const T at(size_t index) const
    {
        if (index >= m_objects.size())
        {
            std::stringstream ss;
            ss << "NamedStorage out of range: max=" << m_objects.size()-1 << " requested=" << index;
            throw std::out_of_range(ss.str());
        }

        return m_objects.at(index);
    }

    auto begin()
    {
        return m_objects.begin();
    }

    auto begin() const
    {
        return m_objects.begin();
    }

    auto end()
    {
        return m_objects.end();
    }

    auto end() const
    {
        return m_objects.end();
    }

    void addListener(INamedStorageListener *listener)
    {
        auto iter = std::find(m_listeners.begin(), m_listeners.end(), listener);
        if (iter == m_listeners.end())
        {
            m_listeners.push_back(listener);
        }
    }

    void removeListener(INamedStorageListener *listener)
    {
        auto iter = std::find(m_listeners.begin(), m_listeners.end(), listener);
        if (iter != m_listeners.end())
        {
            m_listeners.erase(iter);
        }        
    }

protected:
    
    void notifyAll(ssize_t index = -1, INamedStorageListener::NotificationType t = 
        INamedStorageListener::NotificationType::UNSPECIFIED)
    {
        for(auto const listener : m_listeners)
        {
            if (listener != nullptr)
            {
                listener->notify(index, t);
            }
        }
    }

    std::vector<INamedStorageListener*> m_listeners;
    std::vector<T> m_objects;
    std::unordered_map<std::string, size_t> m_nameToIndex;
};



};
