// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <numeric>

namespace LunaCore::Algebra
{

/** A vector class */
template<typename T>
class Vector
{
public:
    Vector() = default;
    Vector(std::size_t sz, T value = 0) : m_vec(sz, value) {}

    /** make all the entries of the vector zero. */
    void zero() noexcept
    {
        m_vec.assign(m_vec.size(), 0);
    }

    /** return the number of entries in the vector */
    [[nodiscard]] constexpr auto size() const noexcept
    {
        return m_vec.size();
    }

    /** resize the vector to the given size.
        make any created entries zero.
    */
    constexpr void resize(std::size_t sz) noexcept
    {
        m_vec.resize(sz, 0);
    }

    constexpr auto begin() const noexcept
    {
        return m_vec.begin();
    }

    constexpr auto end() const noexcept
    {
        return m_vec.end();
    }

    constexpr auto begin() noexcept
    {
        return m_vec.begin();
    }

    constexpr auto end() noexcept
    {
        return m_vec.end();
    }


    [[nodiscard]] constexpr T& at(std::size_t index) { return m_vec.at(index); }
    [[nodiscard]] constexpr const T& at(std::size_t index) const { return m_vec.at(index); }

protected:
    std::vector<T> m_vec;
};

template<class T>
Vector<T> operator-(const Vector<T> &v1, const Vector<T> &v2)
{
    const std::size_t N = v1.size();
    assert(N == v2.size());

    auto result = v1;

    for(std::size_t idx=0; idx<v1.size(); idx++)
    {
        result.at(idx) -= v2.at(idx);
    }
    return result;
}

template<class T>
Vector<T> operator+(const Vector<T> &v1, const Vector<T> &v2)
{
    const std::size_t N = v1.size();
    assert(N == v2.size());

    auto result = v1;

    for(std::size_t idx=0; idx<v1.size(); idx++)
    {
        result.at(idx) += v2.at(idx);
    }
    return result;
}

template<class T>
Vector<T> operator*(T factor, const Vector<T> &v2)
{
    const std::size_t N = v2.size();
    Vector<T> result = v2;

    for(auto &value : result)
    {
        value *= factor;
    }
    return result;
}

template<class T>
Vector<T> operator*(const Vector<T> &v1, const Vector<T> &v2)
{
    const std::size_t N = v1.size();
    assert(v2.size() == N);
    Vector<T> result = v1;

    for(std::size_t idx=0; idx<v1.size(); idx++)
    {
        result.at(idx) = v1.at(idx) * v2.at(idx);
    }
    return result;
}

/** calculate the L2 norm of a vector. */
template<typename T>
constexpr auto norm2(const Vector<T> &vec) noexcept
{
    return std::inner_product(vec.begin(), vec.end(), vec.begin(), 0.0f);
}

/** calculate the dot/inner product of two vectors. */
template<typename T>
constexpr auto dot(const Vector<T> &vec1, const Vector<T> &vec2) noexcept
{
    return std::inner_product(vec1.begin(), vec1.end(), vec2.begin(), 0.0f);
}

};