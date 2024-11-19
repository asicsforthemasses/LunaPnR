#pragma once

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <functional>

namespace LunaCore
{

/** An ID type with tag. The C++ type checker uses the tag
    to differentiate between different ID types.

    Conversions to std::size_t and int are supported so
    IDs can be used to index containers.

 */
template<typename tag>
class MyID
{
public:
    using ValueType = int32_t;
    static constexpr ValueType INVALID_VALUE = -1;

    /** return an invalid ID */
    static constexpr MyID invalid() noexcept
    {
        return MyID();
    }

    explicit constexpr MyID() = default;

    /** no automatic construction from other types. */
    explicit constexpr MyID(ValueType id) : m_id(id) {}

    /** allow conversion to bool to check for validity */
    explicit constexpr operator bool() const { return m_id != INVALID_VALUE; }

    /** check if the ID is valid */
    constexpr bool isValid() const { return m_id != INVALID_VALUE; }

    /** allow convertion to std::size_t so IDs can index STL containers */
    explicit constexpr operator std::size_t() const { return static_cast<std::size_t>(m_id); }

    explicit constexpr operator int() const { return static_cast<int>(m_id); }

    /** allow hashing of IDs */
    friend std::hash< MyID<tag> >;

    // comparison operators
    constexpr bool operator==(const MyID<tag>& other) const noexcept
    {
        return m_id == other.m_id;
    }

    constexpr bool operator!=(const MyID<tag>& other) const noexcept
    {
        return m_id != other.m_id;
    }

    constexpr bool operator<(const MyID<tag>& other) const noexcept
    {
        return m_id < other.m_id;
    }

    friend std::ostream& operator<< <>(std::ostream& out, const MyID<tag>& rhs);

protected:
    ValueType m_id{INVALID_VALUE};
};

};

template<typename tag>
std::ostream& operator<<(std::ostream& out, const LunaCore::MyID<tag>& id)
{
    out << id.m_id;
    return out;
};

// make sure we can hash the ID
namespace std
{

    template<typename tag>
    struct hash<LunaCore::MyID<tag>>
    {
        using ValueType = LunaCore::MyID<tag>::ValueType;

        std::size_t operator()(const LunaCore::MyID<tag> id) const noexcept
        {
            return std::hash<ValueType>()(id.m_id);
        }

    };

}; // Namespace STD
