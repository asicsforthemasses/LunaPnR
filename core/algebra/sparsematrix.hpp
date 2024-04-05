// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <cassert>
#include <list>
#include <vector>
#include <algorithm>
#include <iterator>

namespace LunaCore::Algebra
{

template<class T>
class SparseMatrix
{
public:

    struct RowEntry
    {
        T m_value{0.0f};
        std::size_t m_col{0};
    };

    using RowEntries = std::vector<RowEntry>;
    using RowEntryIterator = RowEntries::const_iterator;
    using MatrixType = std::vector<RowEntries>;
    using RowIterator = MatrixType::const_iterator;

    constexpr SparseMatrix() = default;

    constexpr SparseMatrix(std::size_t rowCount)
    {
        m_rows.resize(rowCount);
    }

    [[nodiscard]] const T& at(std::size_t row, std::size_t col) const
    {
        if (row >= m_rows.size())
        {
            throw std::out_of_range("row does not exist");
        }

        auto &matrixRow = m_rows.at(row);

        auto iter = std::lower_bound(
            matrixRow.begin(),
            matrixRow.end(),
            col,
            [&](auto const &rowEntry1, auto value)
            {
                return rowEntry1.m_col < value;
            }
        );

        // item not found, insert a new row item
        if (iter == matrixRow.end())
        {
            throw std::out_of_range("item does not exist");
        }

        if (iter->m_col != col)
        {
            throw std::out_of_range("item does not exist");
        }

        return iter->m_value;
    }

    [[nodiscard]] T& at(std::size_t row, std::size_t col)
    {
        if (row >= m_rows.size())
        {
            m_rows.resize(row+1);
        }

        auto &matrixRow = m_rows.at(row);

        auto iter = std::lower_bound(
            matrixRow.begin(),
            matrixRow.end(),
            col,
            [&](auto const &rowEntry1, auto value)
            {
                return rowEntry1.m_col < value;
            }
        );

        // item not found, insert a new row item
        if (iter == matrixRow.end())
        {
            auto &item = matrixRow.emplace_back();
            item.m_col = col;
            m_entries++;
            return item.m_value;
        }

        if (iter->m_col != col)
        {
            // exact column not found,
            // we need to insert it
            // iter points to the item
            // with a larger m_col value
            iter = matrixRow.insert(iter, RowEntry{});
            iter->m_col = col;
            m_entries++;
        }

        return iter->m_value;
    }

    [[nodiscard]] constexpr auto nonzeroCount() const noexcept
    {
        return m_entries;
    }

    [[nodiscard]] constexpr auto rowCount() const noexcept
    {
        return m_rows.size();
    }

    [[nodiscard]] constexpr auto rowEntryCount(std::size_t row) const
    {
        return m_rows.at(row).size();
    }

    struct RowColValue
    {
        std::size_t m_row{0};
        std::size_t m_col{0};
        T           m_value{0.0f};
    };

    class ConstIterator
    {
    public:
        using value_type = RowColValue;
        using iterator_tag = std::forward_iterator_tag;
        using pointer = const value_type*;
        using reference = const value_type&;
        using difference_type = std::ptrdiff_t;

        ConstIterator(const MatrixType &matrix, bool atEnd = false)
            : m_matrix(matrix), m_atEnd(atEnd)
        {
            if (!atEnd)
            {
                m_rowIterator = m_matrix.begin();
                m_itemIter = m_rowIterator->begin();

                m_item.m_row   = 0;
                m_item.m_col   = m_itemIter->m_col;
                m_item.m_value = m_itemIter->m_value;
            }
            else
            {
                m_rowIterator = m_matrix.end();
            }
        }

        const RowColValue& operator*() const
        {
            return m_item;
        }

        const RowColValue* operator->() const
        {
            return &m_item;
        }

        ConstIterator operator++()
        {
            m_itemIter++;
            if (m_itemIter == m_rowIterator->end())
            {
                do
                {
                    m_rowIterator++;
                    m_item.m_row++;

                    if (m_rowIterator == m_matrix.end())
                    {
                        m_atEnd = true;
                        return *this;
                    }

                    m_itemIter = m_rowIterator->begin();
                } while (m_rowIterator->empty());
            }

            m_item.m_col   = m_itemIter->m_col;
            m_item.m_value = m_itemIter->m_value;

            return *this;
        }

        ConstIterator operator++(int)
        {
            ConstIterator tmp = *this;
            ++(*this);
            return tmp;
        }

        constexpr bool operator==(const ConstIterator &other) const
        {
            if (m_atEnd && other.m_atEnd) return true;

            return (m_rowIterator == other.m_rowIterator) &&
                (m_itemIter == other.m_itemIter);
        }

        constexpr bool operator!=(const ConstIterator &other) const
        {
            if (m_atEnd == other.m_atEnd) return false;

            return (m_rowIterator != other.m_rowIterator) ||
                (m_itemIter != other.m_itemIter);
        }

    protected:
        const MatrixType &m_matrix;
        RowIterator      m_rowIterator;
        RowEntryIterator m_itemIter;
        RowColValue      m_item;
        bool             m_atEnd{false};
    };

    auto begin() const
    {
        return ConstIterator(m_rows);
    }

    auto end() const
    {
        return ConstIterator(m_rows, true);
    }

protected:
    MatrixType  m_rows;
    std::size_t m_entries{0};
};

template<typename T>
class Vector
{
public:
    Vector() = default;
    Vector(std::size_t sz, T value = 0) : m_vec(sz, value) {}

    void zero() noexcept
    {
        m_vec.assign(m_vec.size(), 0);
    }

    [[nodiscard]] constexpr auto size() const noexcept
    {
        return m_vec.size();
    }

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
Vector<T> operator*(const SparseMatrix<T> &matrix, const Vector<T> &x)
{
    assert(matrix.rowCount() == x.size());

    Vector<T> result;
    result.resize(x.size());

    std::size_t row = 0;
    float sum = 0.0f;
    for(auto const& matEntry : matrix)
    {
        if (matEntry.m_row != row)
        {
            result.at(row) = sum;
            sum = 0.0f;
            row = matEntry.m_row;
        }
        sum += x.at(matEntry.m_col) * matEntry.m_value;
    }
    result.at(row) = sum;
    return result;
}

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


};