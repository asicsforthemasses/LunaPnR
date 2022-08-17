#pragma once

#include <string>

namespace ChipDB
{
    /** replace {envvar} with its expansion */
    std::string expandEnvironmentVars(const std::string &path);

    bool setEnvironmentVar(const std::string &key, const std::string &value);
    bool unsetEnvironmentVar(const std::string &key);
    std::string getEnvironmentVar(const std::string &key);

    std::string findAndReplace(const std::string &str, const std::string &findMe, const std::string &replaceWithMe);

#if 0
    bool deleteFile(const std::string &path) noexcept;
    bool renameFile(const std::string &oldName, const std::string &newName) noexcept;
    std::string generateTempFilename();
#endif
};
