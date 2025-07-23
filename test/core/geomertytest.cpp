
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

BOOST_AUTO_TEST_SUITE(GeometryTest)

BOOST_AUTO_TEST_CASE(various_geometry_tests)
{
    std::cout << "--== GEOMETRY TEST ==--\n";

    ChipDB::Rectangle r1(ChipDB::Rect64{{1,2},{4,4}});
    ChipDB::Rectangle r2(ChipDB::Rect64{{3,1},{7,3}});

    auto overlap = r1.intersect(r2);
    BOOST_CHECK(overlap.has_value());

    auto const& newRect = overlap.value();
    BOOST_CHECK(newRect.left() == 3);
    BOOST_CHECK(newRect.right() == 4);
    BOOST_CHECK(newRect.top() == 3);
    BOOST_CHECK(newRect.bottom() == 2);

    std::cout << "New rectangle is: " << newRect.m_rect << "\n";

    // check for non-overlap
    ChipDB::Rectangle r3(ChipDB::Rect64{{5,1},{7,3}});
    auto overlap2 = r1.intersect(r3);
    BOOST_CHECK(!overlap2.has_value());

    // check abutted rectangles don't overlap
    ChipDB::Rectangle r4(ChipDB::Rect64{{4,1},{7,3}});
    auto overlap3 = r1.intersect(r4);
    BOOST_CHECK(!overlap3.has_value());
};

BOOST_AUTO_TEST_CASE(pin_location_tests)
{
    std::cout << "--== PIN LOCATION TEST ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    auto dffKeyObj = design.m_cellLib->lookupCell("DFFPOSX1");
    BOOST_CHECK(dffKeyObj.isValid());

    auto cellSize = dffKeyObj->m_size;

    auto metal2KeyObj = design.m_techLib->lookupLayer("metal2");
    BOOST_CHECK(metal2KeyObj.isValid());

    auto pitch  = metal2KeyObj->m_pitch;
    auto offset = metal2KeyObj->m_offset;
    auto width  = metal2KeyObj->m_width;

    auto pinPtr = dffKeyObj->m_pins.at(0);
    BOOST_CHECK(pinPtr);

    std::cout << "  checking pin: " << pinPtr->name() << "\n";

    auto const& metal1Layout = pinPtr->m_pinLayout.at("metal1");
    ChipDB::findPinLocations(metal1Layout, cellSize,
        pitch, width, offset);

};

BOOST_AUTO_TEST_SUITE_END()
