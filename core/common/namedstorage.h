#pragma once

#include <type_traits>
#include <sstream>
#include <vector>
#include <string>
#include <unordered_map>

namespace ChipDB {

/** container to store object pointers and provides fast named lookup. */
template <class T>
class NamedStorage
{
public:
    NamedStorage() : m_numberOfObjects(0)
    {
        static_assert(std::is_pointer<T>::value, "Stored object type must be a pointer");
    }

    size_t size() const
    {
        return m_numberOfObjects;
    }

    bool add(const std::string &name, T object)
    {
        auto iter = m_nameToIndex.find(name);
        if (iter == m_nameToIndex.end())
        {
            // no such named object, okay to add!
            m_objects.push_back(object);
            m_nameToIndex[name] = m_objects.size()-1;
            m_numberOfObjects++;
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
                m_numberOfObjects--;
                m_objects[index] = nullptr;
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

protected:
    size_t m_numberOfObjects;
    std::vector<T> m_objects;
    std::unordered_map<std::string, size_t> m_nameToIndex;
};



};
