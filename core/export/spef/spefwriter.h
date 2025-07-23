// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <iostream>
#include <memory>
#include <string>
#include "database/database.h"

/** Parasitic file output for OpenSTA
 *  For now, it only does manhattan segment lengths from source to sink,
 *  all tied at the source.
 *
*/
namespace LunaCore::SPEF
{
    bool write(std::ostream &os, const std::shared_ptr<ChipDB::Module> mod);
    bool write(const std::string &filename, const std::shared_ptr<ChipDB::Module> mod);
};
