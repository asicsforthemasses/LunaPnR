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

BOOST_AUTO_TEST_SUITE(CommonTests)

BOOST_AUTO_TEST_CASE(check_toupper)
{
    std::cout << "--== CHECK TOUPPER ==--\n";
    BOOST_CHECK(LunaCore::toupper("1234AbCdEf") == std::string("1234ABCDEF"));
}

BOOST_AUTO_TEST_CASE(check_tolower)
{
    std::cout << "--== CHECK TOLOWER ==--\n";
    BOOST_CHECK(LunaCore::tolower("1234AbCdEf") == std::string("1234abcdef"));
}

BOOST_AUTO_TEST_CASE(check_find_and_replace)
{
    std::cout << "--== CHECK FIND AND REPLACE ==--\n";

    auto result = LunaCore::findAndReplace("The quick brown fox jumped over the lazy cow",
        "fox", "cow");

    BOOST_CHECK(result == "The quick brown cow jumped over the lazy cow");

    result = LunaCore::findAndReplace(result, "cow", "snake");

    BOOST_CHECK(result == "The quick brown snake jumped over the lazy snake");
}

BOOST_AUTO_TEST_CASE(check_replace_keys_in_braces)
{
    std::unordered_map<std::string, std::string> container;

    container["PDKROOT"] = "Nile";
    container["PROJECTROOT"] = "Rodgers";

    std::string src = "{PDKROOT} {PROJECTROOT} is quite funky! {BOOTSY_BABY}";

    auto result = LunaCore::replaceKeysInBraces(container, src);

    BOOST_CHECK(result == "Nile Rodgers is quite funky! {BOOTSY_BABY}");
}

BOOST_AUTO_TEST_SUITE_END()
