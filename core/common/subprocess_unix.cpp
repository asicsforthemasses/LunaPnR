/*
  LunaPnR Source Code

  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

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

ChipDB::Subprocess::TempFileDescriptor::~TempFileDescriptor()
{
    close();
    remove(m_name.c_str());
}

std::unique_ptr<ChipDB::Subprocess::TempFileDescriptor> ChipDB::Subprocess::createTempFile()
{
    auto descriptor = std::make_unique<ChipDB::Subprocess::TempFileDescriptor>();
    descriptor->m_name = "/tmp/luna_XXXXXX.tcl";
    int result = mkstemps(&descriptor->m_name.at(0), 4);
    close(result);

    descriptor->m_stream.open(descriptor->m_name);

    return descriptor;
}
