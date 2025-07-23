// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "fileutils.h"

#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <regex>
#include <filesystem>
#include "strutils.hpp"

#ifdef __unix__
#include <unistd.h>

std::unique_ptr<LunaCore::TempFileDescriptor> LunaCore::createTempFile(const std::string &extension)
{
    auto descriptor = std::make_unique<LunaCore::TempFileDescriptor>();
    descriptor->m_name = "/tmp/luna_XXXXXX.";
    descriptor->m_name.append(extension);
    int result = mkstemps(&descriptor->m_name.at(0), extension.size()+1);
    close(result);

    descriptor->m_stream.open(descriptor->m_name);

    return descriptor;
}


LunaCore::TempFileDescriptor::~TempFileDescriptor()
{
    close();
    remove(m_name.c_str());
}
#else
#error Windows or OSX not implemented yet
#endif

namespace LunaCore
{

std::string expandEnvironmentVars(const std::string &path)
{
    std::regex envmatcher(R"(\{(.*?)\})");    // matches everything within {}

    std::string inputPath = path;

    auto matches_begin = std::sregex_iterator(inputPath.begin(),
        inputPath.end(), envmatcher);
    auto matches_end   = std::sregex_iterator();

    auto outputPath = inputPath;
    for(auto iter = matches_begin; iter != matches_end; iter++)
    {
        auto match = *iter;
        if (match.size() < 2) continue;

        auto const& completeMatch = match.str(0);
        auto const& envVarName    = match.str(1);

        auto envVarValue = getEnvironmentVar(envVarName);
        if (!envVarValue.empty())
        {
            outputPath = findAndReplace(outputPath, completeMatch, envVarValue);
        }
    }
    return outputPath;
}

bool setEnvironmentVar(const std::string &key, const std::string &value)
{
    if (setenv(key.c_str(), value.c_str(), 1 /* always overwrite */) == 0)  // NOLINT(clang-analyzer-cplusplus.StringChecker)
    {
        return true;
    }
    return false;
}

std::string getEnvironmentVar(const std::string &key)
{
    auto envVarPtr = getenv(key.c_str());
    if (envVarPtr == nullptr)
    {
        return "";
    }
    return envVarPtr;
}

bool unsetEnvironmentVar(const std::string &key)
{
    return unsetenv(key.c_str()) == 0; // NOLINT(clang-analyzer-cplusplus.StringChecker)
}

bool deleteFile(const std::string &filename) noexcept
{
    std::filesystem::path path(filename);
    std::error_code ec;

    std::filesystem::remove(path, ec);
    if (!ec)
    {
        return false;
    }

    return true;
}

bool renameFile(const std::string &oldName, const std::string &newName) noexcept
{
    std::filesystem::path from(oldName);
    std::filesystem::path to(oldName);
    std::error_code ec;

    std::filesystem::rename(from, to, ec);

    if (!ec)
    {
        return false;
    }

    return false;
}

bool copyFile(const std::string &srcName, const std::string &copyName) noexcept
{
    std::filesystem::path from(srcName);
    std::filesystem::path to(copyName);
    std::error_code ec;

    auto const options = std::filesystem::copy_options::overwrite_existing;

    std::filesystem::copy_file(from, to, options, ec);

    if (!ec)
    {
        return false;
    }

    return false;
}

bool fileExists(const std::string &filename) noexcept
{
    std::filesystem::path path(filename);
    return std::filesystem::exists(filename);
}

};
