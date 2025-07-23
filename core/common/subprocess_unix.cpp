// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "subprocess.h"

#include <unistd.h>
#include <array>

bool ChipDB::Subprocess::run(const std::string &command, std::function<void(const std::string&)> lineCallback)
{
    FILE *pipe = popen(command.c_str(), "r");
    if (pipe == nullptr)
    {
        return false;
    }

    std::string strBuffer;
    std::array<char,256> buffer;

    while(fgets(&buffer.at(0), buffer.size(), pipe) != nullptr)
    {
        strBuffer += &buffer.at(0);
        if (!strBuffer.empty() && strBuffer.back() == '\n')
        {
            lineCallback(strBuffer);
            strBuffer.clear();
        }
        if (!strBuffer.empty())
        {
            lineCallback(strBuffer);
        }
    }

    if (pclose(pipe) < 0)
    {
        return false;
    }

    return true;
};
