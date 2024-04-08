// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "database/database.h"
#include "quickplace_impl.hpp"

namespace LunaCore::QuickPlace
{

bool place(Database &db, ChipDB::Module &mod)
{
    PlacerImpl placer;
    return placer.place(db, mod);
}

};