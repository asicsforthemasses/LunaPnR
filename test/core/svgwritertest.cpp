// SPDX-FileCopyrightText: 2021-2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(SVGWriterTest)

BOOST_AUTO_TEST_CASE(can_write_svg)
{
    std::cout << "--== SVG WRITER ==--\n";

    std::ofstream ofile("test/files/results/svgwriter.svg");

    LunaCore::SVG::Writer svg(ofile, 2000, 2000);
    svg.setFillColour(0x000000A0);
    svg.setStrokeWidth(10);
    svg.drawCircle({1000,1000}, 500);

    svg.setFillColour(0xFF0000A0);
    svg.drawRectangle({0, 0}, {500, 500});

    svg.setStrokeWidth(40);
    svg.setStrokeColour(0xF0F000FF);
    svg.drawLine({400,400},{900,900});

    svg.setFontFamily("Arial, Helvetica, sans-serif");
    svg.setFontSize(300);
    svg.setStrokeColour(0xF000F0FF);
    svg.drawCenteredText({1000,1000}, "Garfield");
}

BOOST_AUTO_TEST_SUITE_END()
