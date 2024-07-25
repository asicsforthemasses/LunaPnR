#pragma once
#include <vector>
#include <sstream>
#include "id.hpp"

namespace LunaCore
{

/** vector with ID type lookup
    KeyType must be convertable to size_t.
*/
template<typename KeyType, typename T>
class KeyVector
{
public:

    using const_reference = std::vector<T>::const_reference;
    using reference = std::vector<T>::reference;
    using iterator = std::vector<T>::iterator;
    using const_iterator = std::vector<T>::const_iterator;
    using const_reverse_iterator = std::vector<T>::const_reverse_iterator;

    const_iterator begin() const { return m_data.begin(); }
    const_iterator end() const { return m_data.end(); }
    const_iterator cbegin() const { return m_data.cbegin(); }
    const_iterator cend() const { return m_data.cend(); }

    const_reverse_iterator rbegin() const { return m_data.rbegin(); }
    const_reverse_iterator rend() const { return m_data.rend(); }

    // constructor
    template<typename... Args>
    KeyVector(Args&&... args) : m_data(std::forward<Args>(args)...) {}

    /** Get a const reference to the item using its key.
        Unlike std::vector, KeyVector throws an std::out_of_range exception.
    */
    const_reference operator[](const KeyType idx) const
    {
        auto index = size_t(idx);

        if (index >= m_data.size())
        {
            std::stringstream ss;
            ss << "KeyVector out of range, size = " << m_data.size() << " index = " << index;
            throw std::out_of_range(ss.str());
        }

        return m_data[index];
    }

    /** return the number of elemens in the vector */
    std::size_t size() const { return m_data.size(); }

    bool empty() const { return m_data.empty(); }

    /** return true if the vector contains an item with the specified key. */
    bool contains(const KeyType key) const { return size_t(key) < m_data.size(); }

    /** find the item with its key */
    const_iterator find(const KeyType key) const
    {
        if (size_t(key) < m_data.size())
        {
            return m_data.begin() + size_t(key);
        }
        else
        {
            return m_data.end();
        }
    }

    auto& back()
    {
        return m_data.back();
    }

    auto const& back() const
    {
        return m_data.back();
    }

    auto& front()
    {
        return m_data.front();
    }

    auto const& front() const
    {
        return m_data.front();
    }

    template<typename... Args>
    void push_back(Args&&... args) { m_data.push_back(std::forward<Args>(args)...); }

    template<typename... Args>
    void emplace_back(Args&&... args) { m_data.emplace_back(std::forward<Args>(args)...); }

    template<typename... Args>
    void resize(Args&&... args) { m_data.resize(std::forward<Args>(args)...); }

    void clear() { m_data.clear(); }

    std::size_t capacity() { return m_data.capacity(); }

    void shrink_to_fit() { m_data.shrink_to_fit(); }

    iterator begin() { return m_data.begin(); }
    iterator end() { return m_data.end(); }

    reference operator[](const KeyType key)
    {
        auto index = size_t(key);

        if (index >= m_data.size())
        {
            std::stringstream ss;
            ss << "KeyVector out of range, size = " << m_data.size() << " index = " << index;
            throw std::out_of_range(ss.str());
        }

        return m_data[index];
    }

    iterator find(const KeyType key)
    {
        if (size_t(key) < m_data.size())
        {
            return m_data.begin() + size_t(key);
        }
        else
        {
            return m_data.end();
        }
    }

    void insert(const KeyType key, const T value)
    {
        if (size_t(key) >= m_data.size())
        {
            m_data.resize(size_t(key) + 1, KeyType::INVALID_VALUE);
        }

        operator[](key) = value;
    }

protected:
    std::vector<T> m_data;
};

};
