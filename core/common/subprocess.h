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

    struct TempFileDescriptor
    {
        virtual ~TempFileDescriptor();
        std::string   m_name;
        std::ofstream m_stream;

        bool good() const
        {
            return m_stream.good();
        }

        void close()
        {
            if (m_stream.is_open()) m_stream.close();
        }
    };

    bool run(const std::string &command, std::function<void(const std::string&)> lineCallback);
    std::unique_ptr<TempFileDescriptor> createTempFile();

};
