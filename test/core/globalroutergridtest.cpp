// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(GlobalRouterGridTest)

BOOST_AUTO_TEST_CASE(check_grid)
{
    std::cout << "--== CHECK GLOBAL ROUTER GRID ==--\n";
    
    LunaCore::GlobalRouter::Grid grid(20,10,{1,1});

    BOOST_CHECK(grid.height() == 10);
    BOOST_CHECK(grid.width() == 20);

    // check that all cells have reset flags
    for(auto const& cell : grid.gcells())
    {
        BOOST_CHECK(!cell.isBlocked());
        BOOST_CHECK(!cell.isMarked());
        BOOST_CHECK(!cell.isTarget());
        BOOST_CHECK(!cell.isReached());
        BOOST_CHECK(!cell.isSource());
        BOOST_CHECK(cell.isValid());
    }

    // check that an out-of-bounds cell access
    // returns an invalid cell
    BOOST_CHECK(!grid.at(-1,-1).isValid());
}


BOOST_AUTO_TEST_SUITE_END()