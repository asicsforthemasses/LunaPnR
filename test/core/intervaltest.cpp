// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(IntervalTest)

BOOST_AUTO_TEST_CASE(various_interval_tests)
{
    std::cout << "--== INTERVAL TEST ==--\n";

    ChipDB::Interval I1{10,20};

    BOOST_CHECK(I1.overlap({10,20}));

    BOOST_CHECK(I1.contains(11));
    BOOST_CHECK(!I1.contains(9));

    BOOST_CHECK(I1.overlap({7,20}));
    BOOST_CHECK(I1.overlap({7,40}));
    BOOST_CHECK(I1.overlap({12,18}));
    BOOST_CHECK(!I1.overlap({21,28}));
    BOOST_CHECK(!I1.overlap({1,3}));
    BOOST_CHECK((ChipDB::Interval{10,20} < ChipDB::Interval{30,40}));
    BOOST_CHECK(!(ChipDB::Interval{10,20} > ChipDB::Interval{30,40}));

    auto m1 = I1.merge({0,10});
    BOOST_CHECK(m1.x1 == 0);
    BOOST_CHECK(m1.x2 == 20);

    auto c1 = I1.common({15,18});
    BOOST_CHECK(c1.x1 == 15);
    BOOST_CHECK(c1.x2 == 18);
};

bool checkIntervals(const ChipDB::IntervalList &list,
    const std::vector<ChipDB::Interval> &check)
{
    if (list.size() != check.size()) return false;

    auto iter1 = list.begin();
    auto iter2 = check.begin();

    while(iter1 != list.end())
    {
        if (*iter1 != *iter2) return false;
        iter1++;
        iter2++;
    }
    return true;
}

BOOST_AUTO_TEST_CASE(various_interval_list_tests)
{
    std::cout << "--== INTERVAL LIST TEST ==--\n";

    ChipDB::IntervalList myIntervals;
    BOOST_CHECK(myIntervals.size() == 0);

    myIntervals.addInterval({10,20});
    BOOST_CHECK(myIntervals.size() == 1);

    myIntervals.addInterval({30,40});
    BOOST_CHECK(myIntervals.size() == 2);

    myIntervals.addInterval({1,5});
    BOOST_CHECK(myIntervals.size() == 3);

    const std::vector<ChipDB::Interval> check =
    {
        {1,5},{10,20},{30,40}
    };

    BOOST_CHECK(checkIntervals(myIntervals, check));

    myIntervals.addInterval({40,50});

    const std::vector<ChipDB::Interval> check2 =
    {
        {1,5},{10,20},{30,50}
    };

    BOOST_CHECK(checkIntervals(myIntervals, check2));

    myIntervals.addInterval({0,100});

    const std::vector<ChipDB::Interval> check3 =
    {
        {0,100}
    };

    BOOST_CHECK(checkIntervals(myIntervals, check3));

    myIntervals.clear();
    BOOST_CHECK(myIntervals.size() == 0);

    myIntervals.addInterval({10,20});
    myIntervals.addInterval({30,40});
    myIntervals.addInterval({50,60});
    myIntervals.addInterval({120,130});
    myIntervals.addInterval({0,100});

    const std::vector<ChipDB::Interval> check4 =
    {
        {0,100},{120,130}
    };

    BOOST_CHECK(checkIntervals(myIntervals, check4));
};

BOOST_AUTO_TEST_SUITE_END()
