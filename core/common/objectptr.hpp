// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only
//
// A non-owning object pointer.
//

#pragma once
#include <type_traits>
#include <memory>

namespace LunaCore
{

template<typename T>
class ObjectPtr
{
public:
    constexpr ObjectPtr() noexcept : m_ptr(nullptr) {}
    constexpr ObjectPtr(std::nullptr_t) noexcept : m_ptr(nullptr) {}
    constexpr ObjectPtr(T *ptr) noexcept : m_ptr(ptr) {}

    constexpr T* get() const noexcept
    {
        return m_ptr;
    }

    constexpr T& operator*() const noexcept
    {
        return *m_ptr;
    }

    constexpr explicit operator T *() const noexcept
    {
        return m_ptr;
    }

    constexpr T* operator->() const noexcept
    {
        return m_ptr;
    }

    constexpr explicit operator bool() const noexcept
    {
        return m_ptr != nullptr;
    }

    constexpr bool operator!() const noexcept
    {
        return m_ptr == nullptr;
    }

    constexpr void reset(T *ptr = nullptr) noexcept
    {
        m_ptr = ptr;
    }

    friend constexpr bool
    operator==(ObjectPtr const &lhs, ObjectPtr const &rhs) noexcept
    {
        return lhs.ptr == rhs.ptr;
    }

    friend constexpr bool
    operator!=(ObjectPtr const &lhs, ObjectPtr const &rhs) noexcept
    {
        return !(lhs == rhs);
    }

protected:
    T *m_ptr;
};

};

namespace std
{
    /// Allow hashing object_ptrs so they can be used as keys in unordered_map
    template <typename T> struct hash<LunaCore::ObjectPtr<T>> {
        constexpr size_t operator()(LunaCore::ObjectPtr<T> const &p) const
            noexcept {
            return hash<T *>()(p.get());
        }
    };

    /// Do a static_cast with object_ptr
    template <typename To, typename From>
    typename std::enable_if<
        sizeof(decltype(static_cast<To *>(std::declval<From *>()))) != 0,
        LunaCore::ObjectPtr<To>>::type
    static_pointer_cast(LunaCore::ObjectPtr<From> p)
    {
        return static_cast<To *>(p.get());
    }

    /// Do a dynamic_cast with object_ptr
    template <typename To, typename From>
    typename std::enable_if<
        sizeof(decltype(dynamic_cast<To *>(std::declval<From *>()))) != 0,
        LunaCore::ObjectPtr<To>>::type
    dynamic_pointer_cast(LunaCore::ObjectPtr<From> p)
    {
        return dynamic_cast<To *>(p.get());
    }

};
