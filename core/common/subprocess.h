// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstdlib>
#include <cstdio>
#include <string>
#include <fstream>
#include <memory>
#include <functional>

namespace ChipDB::Subprocess
{
    bool run(const std::string &command, std::function<void(const std::string&)> lineCallback);
};
