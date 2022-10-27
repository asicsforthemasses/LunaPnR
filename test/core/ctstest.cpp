// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"
#include "tinysvgpp/src/tinysvgpp.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(CTSTest)

TinySVGPP::Point toSVGPoint(const ChipDB::Coord64 &p)
{
    return {static_cast<float>(p.m_x), static_cast<float>(p.m_y)};
}


BOOST_AUTO_TEST_CASE(check_cts)
{
    std::cout << "--== CHECK CTS ==--\n";

    LunaCore::CTS::MeanAndMedianCTS cts;

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());
    
    std::ifstream leffile2("test/files/iit_stdcells_extra/fake_ties018.lef");
    BOOST_REQUIRE(leffile2.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile2));

    std::ifstream verilogfile("test/files/verilog/femtorv32_quark.v");
    BOOST_REQUIRE(verilogfile.good());

    BOOST_REQUIRE(ChipDB::Verilog::Reader::load(design, verilogfile));
    
    auto mod = design.m_moduleLib->lookupModule("FemtoRV32");
    BOOST_REQUIRE(mod.isValid());

    auto netlist = mod->m_netlist;
    BOOST_REQUIRE(netlist);

    BOOST_CHECK(!cts.generateTree("clk_doesnt_exist", *netlist));
    BOOST_CHECK(!cts.generateTree("clk", *netlist));  // fails because cells have not been placed

    // read placement of cells using DEF reader
    std::ifstream deffile("test/files/def/femtorv32_quark.def");
    BOOST_REQUIRE(deffile.good());

    BOOST_REQUIRE(ChipDB::DEF::Reader::load(design, deffile));

    auto tree = cts.generateTree("clk", *netlist);
    BOOST_REQUIRE(tree);

    TinySVGPP::Canvas canvas;
    using SVGPoint = TinySVGPP::Point;
    
    TinySVGPP::Viewport vp;
    vp.canvasWidth = 2000;
    vp.canvasHeight = 2000;

    vp.xmin = 0;
    vp.xmax = 100000;
    vp.ymin = 0;
    vp.ymax = 100000;
    vp.flipY = true;

    canvas.setSize(vp.canvasWidth, vp.canvasHeight);
    canvas.fill("black").stroke("black").rect(0,0, vp.canvasWidth, vp.canvasHeight);

    // draw all terminals    
    for(auto const& segment : tree.value())
    {
        canvas.stroke("green", 1.0).fill("green");
        canvas.circle(vp.toWindow(toSVGPoint(segment.m_start)), 2);
        if (segment.m_insKey != ChipDB::ObjectNotFound)
        {
            canvas.stroke("red", 1.0).fill("red");
        }

        canvas.circle(vp.toWindow(toSVGPoint(segment.m_end)), 2);

        canvas.stroke("yellow", 2.0).fill("yellow");
        canvas.line(
            SVGPoint{vp.toWindow(toSVGPoint(segment.m_start))},
            SVGPoint{vp.toWindow(toSVGPoint(segment.m_end))}
        );
    }

    std::ofstream svgfile("test/files/results/cts.svg");
    canvas.toSVG(svgfile);

    // collect all the end points

    std::list<LunaCore::CTS::MeanAndMedianCTS::SegmentIndex> endNodes;
    std::size_t index=0;
    for(auto const& segment : tree.value())
    {
        if (segment.m_insKey != ChipDB::ObjectNotFound)
        {
            endNodes.push_back(index);            
        }
        index++;
    }

    // check that the number of end nodes we found
    // is equal to the number of sinks on the clk net
    auto clkNet = netlist->lookupNet("clk");
    BOOST_REQUIRE(clkNet.isValid());
    BOOST_CHECK(endNodes.size() == (clkNet->numberOfConnections() - 1));

    ChipDB::CoordType minL = std::numeric_limits<ChipDB::CoordType>::max();
    ChipDB::CoordType maxL = std::numeric_limits<ChipDB::CoordType>::min();
    int minSegs = std::numeric_limits<int>::max();
    int maxSegs = std::numeric_limits<int>::min();
    int minSegIndex = std::numeric_limits<int>::max();
    int maxSegIndex = std::numeric_limits<int>::min();

    for(auto const& segmentIndex : endNodes)
    {
        int segs = 0;
        auto index = segmentIndex;

        ChipDB::CoordType lenToSink = 0;
        while (index >= 0)
        {
            auto const& seg = tree.value().at(index);
            auto L = seg.m_start.manhattanDistance(seg.m_end);
            lenToSink += L;
            index = seg.m_parent;
            segs++;
        }

        minL = std::min(minL, lenToSink);
        maxL = std::max(maxL, lenToSink);

        if (minSegs > segs)
        {
            minSegs = segs;
            minSegIndex = segmentIndex;
        }

        if (maxSegs < segs)
        {
            maxSegs = segs;
            maxSegIndex = segmentIndex;
        }
    }

    std::cout << "  min : " << minL << "  max : " << maxL << "\n";
    std::cout << "  min segs : " << minSegs << "  max segs : " << maxSegs << "\n";
    std::cout << "  min segs index : " << minSegIndex << "  max segs index : " << maxSegIndex << "\n";

    // write out the min and max routes
    TinySVGPP::Canvas canvas2;
    canvas2.setSize(vp.canvasWidth, vp.canvasHeight);
    canvas2.fill("black").stroke("black").rect(0,0, vp.canvasWidth, vp.canvasHeight);

    std::cout << "Checkpoint #0\n";

    int segIndex = minSegIndex;
    while (segIndex >= 0)
    {
        BOOST_REQUIRE(segIndex >= 0);
        BOOST_REQUIRE(segIndex < tree.value().size());

        auto const& seg = tree.value().at(segIndex);

        canvas2.stroke("green", 1.0).fill("green");
        canvas2.circle(vp.toWindow(toSVGPoint(seg.m_start)), 2);
        if (seg.m_insKey != ChipDB::ObjectNotFound)
        {
            canvas2.stroke("red", 1.0).fill("red");
        }

        canvas2.circle(vp.toWindow(toSVGPoint(seg.m_end)), 2);

        canvas2.stroke("yellow", 2.0).fill("yellow");
        canvas2.line(
            SVGPoint{vp.toWindow(toSVGPoint(seg.m_start))},
            SVGPoint{vp.toWindow(toSVGPoint(seg.m_end))}
        );

        segIndex = seg.m_parent;
    }

    std::cout << "Checkpoint #1\n";

    std::ofstream svgfile2("test/files/results/cts_minroute.svg");
    canvas2.toSVG(svgfile2);
    svgfile2.close();

    std::cout << "Checkpoint #2\n";

    TinySVGPP::Canvas canvas3;
    canvas3.setSize(vp.canvasWidth, vp.canvasHeight);
    canvas3.fill("black").stroke("black").rect(0,0, vp.canvasWidth, vp.canvasHeight);

    segIndex = maxSegIndex;
    while (segIndex >= 0)
    {
        BOOST_REQUIRE(segIndex >= 0);
        BOOST_REQUIRE(segIndex < tree.value().size());

        auto const& seg = tree.value().at(segIndex);

        canvas3.stroke("green", 1.0).fill("green");
        canvas3.circle(vp.toWindow(toSVGPoint(seg.m_start)), 2);
        if (seg.m_insKey != ChipDB::ObjectNotFound)
        {
            canvas3.stroke("red", 1.0).fill("red");
        }

        canvas3.circle(vp.toWindow(toSVGPoint(seg.m_end)), 2);

        canvas3.stroke("yellow", 2.0).fill("yellow");
        canvas3.line(
            SVGPoint{vp.toWindow(toSVGPoint(seg.m_start))},
            SVGPoint{vp.toWindow(toSVGPoint(seg.m_end))}
        );

        segIndex = seg.m_parent;
    }

    std::cout << "Checkpoint #3\n";

    std::ofstream svgfile3("test/files/results/cts_maxroute.svg");
    canvas3.toSVG(svgfile3);
    svgfile3.close();

}

BOOST_AUTO_TEST_SUITE_END()