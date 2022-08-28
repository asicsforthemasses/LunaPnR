// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <unordered_map>
#include <cstdlib>

namespace LunaCore::CellPlacer2
{

/** Sparse matrix class to function as intermediary for Eigen3 */
struct Matrix
{
    using ColIndex  = int;
    using RowIndex  = int;
    using ValueType = float;
    using RowType   = std::unordered_map<ColIndex, ValueType>;

    Matrix() = default;

    explicit Matrix(std::size_t reserveRows)
    {
        m_rows.reserve(reserveRows);
    }

    std::unordered_map<RowIndex, RowType> m_rows;

    /** returns a reference to a matrix entry at (r,c)
     *  if the entry doesn't exist, it is created and set to zero
     */
    ValueType& operator()(RowIndex r, ColIndex c) noexcept
    {
        auto &row = m_rows[r];
        
        // make sure the value is zero
        // when it is created automatically by
        // std::unordered_map!

        auto iter = row.find(c);
        if (iter == row.end())
        {
            row[c] = 0.0f;
        }

        return row[c];
    }

    /** returns a const reference to a matrix entry at (r,c)
     *  if the entry doesn't exist, an exception is thrown.
     */
    const ValueType& operator()(RowIndex r, ColIndex c) const noexcept
    {
        auto const& row = m_rows.at(r);
        return row.at(c);
    }

    /** returns the number of non-zero items in the matrix */
    std::size_t nonZeroItemCount() const noexcept
    {
        std::size_t nnz = 0;
        for(auto const& row : m_rows)
        {
            nnz += row.second.size();
        }
        return nnz;
    }

    /** returns a reference to a matrix row of the specified index.
     *  if the row doesn't exist, it is created
     */
    RowType& row(RowIndex r) noexcept
    {
        return m_rows[r];
    }

    /** returns a const reference to a matrix row of the specified index.
     *  if the row doesn't exist, an exception is thrown.
     */
    const RowType& row(RowIndex r) const noexcept
    {
        return m_rows.at(r);
    }   

    /** clear/empty the matrix */
    void clear()
    {
        m_rows.clear();
    }

    /** row iterator begin */
    auto begin() const noexcept
    {
        return m_rows.cbegin();
    }

    /** row iterator end */
    auto end() const noexcept
    {
        return m_rows.cend();
    }
};

};