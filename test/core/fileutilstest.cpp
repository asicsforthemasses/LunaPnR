// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(FileUtils)

BOOST_AUTO_TEST_CASE(file_utils_test)
{
    std::cout << "--== CHECK FILE UTILS: envvars ==--\n";
    const std::string envName{"CheckFileUtilsEnvVarTest"};

    BOOST_CHECK(LunaCore::setEnvironmentVar(envName, "1234"));
    BOOST_CHECK(LunaCore::getEnvironmentVar(envName) == "1234");

    std::string fileName{"/usr/local/{CheckFileUtilsEnvVarTest}"};
    auto expandedName = LunaCore::expandEnvironmentVars(fileName);
    BOOST_CHECK(expandedName == "/usr/local/1234");

    BOOST_CHECK(LunaCore::unsetEnvironmentVar(envName));
};

BOOST_AUTO_TEST_CASE(file_utils_test2)
{
    std::cout << "--== CHECK FILE UTILS: find and replace ==--\n";

    auto result1 = LunaCore::findAndReplace("Hello 1234!", "1234", "4321");
    BOOST_CHECK(result1 == "Hello 4321!");

    auto result2 = LunaCore::findAndReplace("Hello 12341234!", "1234", "4321");
    BOOST_CHECK(result2 == "Hello 43214321!");

    auto result3 = LunaCore::findAndReplace("1234Hello!", "1234", "4321");
    BOOST_CHECK(result3 == "4321Hello!");

    auto result4 = LunaCore::findAndReplace("1234Hello1234", "1234", "4321");
    BOOST_CHECK(result4 == "4321Hello4321");
};

BOOST_AUTO_TEST_SUITE_END()