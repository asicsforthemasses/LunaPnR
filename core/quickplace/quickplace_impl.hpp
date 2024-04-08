// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <unordered_map>
#include "database/database.h"
#include "algebra/cgsolver.hpp"

namespace LunaCore::QuickPlace
{

class PlacerImpl
{
public:
    PlacerImpl() = default;

    bool place(Database &db, ChipDB::Module &mod);

protected:
    using NumType = float;
    using RowIndex = std::size_t;

    void addConnectionToSystem(
        ChipDB::InstanceObjectKey ins1Key,
        const ChipDB::Instance& ins1,
        ChipDB::InstanceObjectKey ins2Key,
        const ChipDB::Instance& ins2,
        float weight) noexcept;

    void addConnectionToSystem(
        RowIndex anchorRow,
        ChipDB::InstanceObjectKey ins2Key,
        const ChipDB::Instance& ins2,
        float weight) noexcept;

    Algebra::SparseMatrix<NumType> m_A;
    Algebra::Vector<NumType> m_Bx;
    Algebra::Vector<NumType> m_By;
    std::unordered_map<ChipDB::InstanceObjectKey, RowIndex> m_insKey2row;
};

};
