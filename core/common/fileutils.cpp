#include "fileutils.h"

#include <sstream>
#include <cstdio>
#include <cstdlib>
#include <regex>

std::string ChipDB::expandEnvironmentVars(const std::string &path)
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

        auto envVarValue = ChipDB::getEnvironmentVar(envVarName);
        if (!envVarValue.empty())
        {
            outputPath = ChipDB::findAndReplace(outputPath, completeMatch, envVarValue);
        }        
    }
    return outputPath;
}

bool ChipDB::setEnvironmentVar(const std::string &key, const std::string &value)
{
    if (setenv(key.c_str(), value.c_str(), 1 /* always overwrite */) == 0)  // NOLINT(clang-analyzer-cplusplus.StringChecker)
    {
        return true;
    }
    return false;
}

std::string ChipDB::getEnvironmentVar(const std::string &key)
{
    auto envVarPtr = getenv(key.c_str());
    if (envVarPtr == nullptr)
    {
        return "";
    }
    return envVarPtr;
}

bool ChipDB::unsetEnvironmentVar(const std::string &key)
{
    return unsetenv(key.c_str()) == 0; // NOLINT(clang-analyzer-cplusplus.StringChecker)
}

std::string ChipDB::findAndReplace(const std::string &str, const std::string &findMe, const std::string &replaceWithMe)
{
    std::ostringstream result;
    std::size_t pos = 0;
    std::size_t prevPos;

    while(true)
    {
        prevPos = pos;
        pos = str.find(findMe, pos);
        if (pos == std::string::npos)
            break;
        result << str.substr(prevPos, pos - prevPos);
        result << replaceWithMe;
        pos += findMe.size();
    }

    result << str.substr(prevPos);
    return result.str();
}
