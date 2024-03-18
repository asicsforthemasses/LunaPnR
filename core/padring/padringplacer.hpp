// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "padring.hpp"
#include "configreader.hpp"
#include "database/database.h"

namespace LunaCore::Padring
{

bool read(std::istream &is, Database &db, Padring &padring);

//void place(Database &db);

};