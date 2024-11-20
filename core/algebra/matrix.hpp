// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cassert>
#include <list>
#include <vector>
#include <algorithm>
#include <iterator>
#include <string_view>
#include "../common/strutils.hpp"
#include "vector.hpp"

namespace LunaCore::Algebra
{

/** A dense matrix class */
template<class T>
class Matrix
{
public:
    constexpr Matrix() = default;

    constexpr Matrix(std::size_t rowCount, std::size_t colCount)
    {
        m_data.resize(rowCount*colCount);
    }

    /** resize the matrix to the specified number of rows */
    void resize(std::size_t rows, std::size_t cols)
    {
        m_data.clear();
        m_data.resize(rows*cols);
    }

    /** get the matrix entry at (row, column).
        if it doesn't exist, throw a std::out_of_range exception.
    */
    [[nodiscard]] const T& at(std::size_t row, std::size_t col) const
    {
        if (row >= m_rows)
        {
            throw std::out_of_range("row does not exist");
        }

        if (col >= m_cols)
        {
            throw std::out_of_range("column does not exist");
        }

        std::size_t idx = m_cols * row + col;

        return m_data.at(idx);
    }

    /** get the matrix entry at (row, column).
        if it doesn't exist, create it.
    */
    [[nodiscard]] T& at(std::size_t row, std::size_t col)
    {
        if (row >= m_rows)
        {
            throw std::out_of_range("row does not exist");
        }

        if (col >= m_cols)
        {
            throw std::out_of_range("column does not exist");
        }

        std::size_t idx = m_cols * row + col;

        return m_data.at(idx);
    }

    /** return the number of rows in the matrix. */
    [[nodiscard]] constexpr auto rowCount() const noexcept
    {
        return m_rows;
    }

    /** return the number of columns in the matrix. */
    [[nodiscard]] constexpr auto colCount() const noexcept
    {
        return m_cols;
    }

    [[nodiscard]] T& operator()(std::size_t row, std::size_t col)
    {
        return at(row, col);
    }

    [[nodiscard]] const T& operator()(std::size_t row, std::size_t col) const
    {
        return at(row, col);
    }

    constexpr auto begin() const
    {
        return m_data.cbegin();
    }

    constexpr auto end() const
    {
        return m_data.cend();
    }

    constexpr auto begin()
    {
        return m_data.begin();
    }

    constexpr auto end()
    {
        return m_data.end();
    }

    auto& operator=(const std::string &numbersStr)
    {
        std::vector<T> nums;

        auto numStrVec = LunaCore::split(numbersStr, ',');
        try
        {
            for(auto const& numStr : numStrVec)
            {
                auto num = std::stod(numStr);
                nums.push_back(num);
            }
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            m_data.clear();
            m_rows = 0;
            m_cols = 0;
            return *this;
        }

        if (nums.size() != (m_rows*m_cols))
        {
            throw std::invalid_argument("Not enough data");
        }

        m_data = nums;
        return *this;
    }    

protected:
    std::vector<T> m_data;
    std::size_t m_rows{0};
    std::size_t m_cols{0};
};

/** result = M*x */
template<class T>
Vector<T> operator*(const Matrix<T> &matrix, const Vector<T> &x)
{
    assert(matrix.colCount() == x.size());
    assert(matrix.rowCount() == x.size());

    Vector<T> result(x.size());

    auto matElement    = matrix.begin();
    auto resultElement = result.begin();
    for(std::size_t row=0; row < matrix.rowCount(); row++)
    {
        T sum{0};
        auto vecElement = x.begin();
        for(std::size_t col=0; row < matrix.colCount(); col++)
        {
            sum += *matElement * *vecElement;
            ++matElement;
            ++vecElement;
        }
        *result = sum;
        ++result;
    }
    return result;
}

};

template<typename T>
std::ostream& operator<<(std::ostream &os, const LunaCore::Algebra::Matrix<T> &matrix)
{
    auto iter = matrix.begin();
    for(std::size_t row=0; row < matrix.rowCount(); row++)
    {
        bool first = true;
        for(std::size_t col=0; row < matrix.colCount(); col++)
        {
            if (!first) os << ", ";
            os << *iter;
            ++iter;
        }
        os << "\n";
    }
    return os;
}
