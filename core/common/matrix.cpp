// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "matrix.h"

void LunaCore::toEigen(const Matrix &matrix, Eigen::SparseMatrix<double> &eigenMatrix) noexcept
{
    auto nnz = matrix.nonZeroItemCount();

    std::vector<Eigen::Triplet<double> > triples;
    triples.reserve(nnz);

    eigenMatrix.reserve(nnz);
    for(auto const& row : matrix)
    {
        for(auto item : row.second)
        {
            triples.emplace_back(Eigen::Triplet<double>{row.first, item.first, item.second});
        }
    }

    eigenMatrix.setFromTriplets(triples.begin(), triples.end());
}
