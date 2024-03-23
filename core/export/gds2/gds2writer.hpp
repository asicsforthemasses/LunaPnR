// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <iostream>
#include "database/database.h"

namespace LunaCore::GDS2
{

bool write(std::ostream &os, const Database &database, const std::string &moduleName);

};

namespace LunaCore::GDS2::WriterImpl
{

    void write32(std::ostream &os, uint32_t value);
    void write16(std::ostream &os, uint16_t value);
    void write8(std::ostream &os, uint8_t value);

    // returns actual number of bytes written
    std::size_t write(std::ostream &os, std::string_view view);

    void write(std::ostream &os, const ChipDB::Instance &instance);
};
