/*
    LunaPnR Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

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
