#pragma once

#include <fstream>
#include <memory>
#include <string>

namespace ChipDB
{
    /** replace {envvar} with its expansion */
    std::string expandEnvironmentVars(const std::string &path);

    bool setEnvironmentVar(const std::string &key, const std::string &value);
    bool unsetEnvironmentVar(const std::string &key);
    std::string getEnvironmentVar(const std::string &key);

    std::string findAndReplace(const std::string &str, const std::string &findMe, const std::string &replaceWithMe);

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
    bool fileExists(const std::string &filename) noexcept;
    std::unique_ptr<TempFileDescriptor> createTempFile(const std::string &extension);

};
