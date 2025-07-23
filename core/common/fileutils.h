// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <fstream>
#include <memory>
#include <string>

namespace LunaCore
{
    /** replace {envvar} with its expansion */
    std::string expandEnvironmentVars(const std::string &path);

    bool setEnvironmentVar(const std::string &key, const std::string &value);
    bool unsetEnvironmentVar(const std::string &key);
    std::string getEnvironmentVar(const std::string &key);

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

    bool deleteFile(const std::string &filename) noexcept;
    bool renameFile(const std::string &oldName, const std::string &newName) noexcept;
    bool copyFile(const std::string &srcName, const std::string &copyName) noexcept;
    bool fileExists(const std::string &filename) noexcept;
    std::unique_ptr<TempFileDescriptor> createTempFile(const std::string &extension);

};
