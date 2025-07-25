// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PrimTests)

static const std::string primCheckTree =
R"(
65
0 0 80000 10000 1 43  71600 50000
1 18 106800 120000 1 23  108400 130000
2 64 122000 140000 0
3 13 130800 150000 1 5  151600 150000
4 12 123600 70000 2 32  106000 70000 10  116400 50000
5 3 151600 150000 1 63  143600 160000
6 19 110000 90000 2 12  116400 80000 9  127600 90000
7 11 126000 120000 1 64  126000 130000
8 9 127600 100000 1 11  126000 110000
9 6 127600 90000 2 8  127600 100000 29  174000 90000
10 4 116400 50000 0
11 8 126000 110000 1 7  126000 120000
12 6 116400 80000 1 4  123600 70000
13 22 113200 150000 1 3  130800 150000
14 21 97200 180000 1 54  123600 180000
15 20 95600 140000 1 22  95600 150000
16 24 90800 110000 1 17  90800 100000
17 16 90800 100000 1 19  108400 100000
18 19 108400 110000 1 1  106800 120000
19 17 108400 100000 2 18  108400 110000 6  110000 90000
20 24 90800 130000 1 15  95600 140000
21 22 95600 170000 1 14  97200 180000
22 15 95600 150000 2 13  113200 150000 21  95600 170000
23 1 108400 130000 0
24 27 89200 120000 2 20  90800 130000 16  90800 110000
25 33 71600 140000 1 42  68400 150000
26 36 93200 80000 0
27 28 71600 120000 2 33  71600 130000 24  89200 120000
28 44 71600 110000 1 27  71600 120000
29 9 174000 90000 2 30  174000 110000 31  174000 60000
30 29 174000 110000 0
31 29 174000 60000 0
32 4 106000 70000 0
33 27 71600 130000 1 25  71600 140000
34 38 66800 80000 1 37  71600 90000
35 43 71600 60000 1 38  69200 70000
36 38 86800 70000 2 51  93200 60000 26  93200 80000
37 34 71600 90000 1 44  71600 100000
38 35 69200 70000 2 34  66800 80000 36  86800 70000
39 42 70800 160000 1 50  70800 170000
40 59 123600 230000 2 41  123600 240000 48  141200 230000
41 40 123600 240000 1 61  126000 250000
42 25 68400 150000 1 39  70800 160000
43 0 71600 50000 1 35  71600 60000
44 37 71600 100000 1 28  71600 110000
45 46 70800 190000 1 47  66800 200000
46 50 67600 180000 1 45  70800 190000
47 45 66800 200000 1 52  70000 210000
48 40 141200 230000 0
49 57 141200 260000 0
50 39 70800 170000 1 46  67600 180000
51 36 93200 60000 0
52 47 70000 210000 0
53 57 123600 270000 1 56  126000 280000
54 14 123600 180000 1 55  122800 190000
55 54 122800 190000 2 60  123600 200000 58  140400 190000
56 53 126000 280000 0
57 61 123600 260000 2 53  123600 270000 49  141200 260000
58 55 140400 190000 0
59 62 126000 220000 1 40  123600 230000
60 55 123600 200000 1 62  123600 210000
61 41 126000 250000 1 57  123600 260000
62 60 123600 210000 1 59  126000 220000
63 5 143600 160000 0
64 7 126000 130000 1 2  122000 140000
)";

BOOST_AUTO_TEST_CASE(check_prim_routines)
{
    std::cout << "--== CHECK PRIM ROUTINES ==--\n";

    const std::string src{R"(65 80000 10000  106800 120000  122000 140000  130800 150000  123600 70000  151600 150000  110000 90000  126000 120000  127600 100000  127600 90000  116400 50000  126000 110000  116400 80000  113200 150000  97200 180000  95600 140000  90800 110000  90800 100000  108400 110000  108400 100000  90800 130000  95600 170000  95600 150000  108400 130000  89200 120000  71600 140000  93200 80000  71600 120000  71600 110000  174000 90000  174000 110000  174000 60000  106000 70000  71600 130000  66800 80000  71600 60000  86800 70000  71600 90000  69200 70000  70800 160000  123600 230000  123600 240000  68400 150000  71600 50000  71600 100000  70800 190000  67600 180000  66800 200000  141200 230000  141200 260000  70800 170000  93200 60000  70000 210000  123600 270000  123600 180000  122800 190000  126000 280000  123600 260000  140400 190000  126000 220000  123600 200000  126000 250000  123600 210000  143600 160000  126000 130000)"};

    auto netNodes = LunaCore::Prim::loadNetNodes(src);

    BOOST_REQUIRE(netNodes.size() == 65);

    auto tree = LunaCore::Prim::prim(netNodes);
    BOOST_CHECK(tree.size() == 65);

    std::cout << tree;

    LunaCore::Prim::MSTree checkTree;
    std::stringstream ss(primCheckTree);
    ss >> checkTree;

    BOOST_CHECK(checkTree.size() == 65);

    for(std::size_t idx=0; idx<checkTree.size(); idx++)
    {
        BOOST_CHECK(checkTree.at(idx) == tree.at(idx));
    }

    const std::string filename("test/files/results/PrimCheck.svg");

    std::ofstream ofile(filename);
    if (ofile.good())
    {
        LunaCore::Prim::toSVG(ofile, tree);
        std::cout << "  SVG file written to " << filename << "\n";
    }
    else
    {
        std::cerr << "  Error writing Prim tree to SVG file " << filename << "\n";
    }
}


BOOST_AUTO_TEST_SUITE_END()