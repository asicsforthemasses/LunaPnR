// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"
#include "tinysvgpp.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <variant>
#include <list>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(CTSTest)

TinySVGPP::Point toSVGPoint(const ChipDB::Coord64 &p)
{
    return {static_cast<float>(p.m_x), static_cast<float>(p.m_y)};
}

using SegmentList = LunaCore::CTS::MeanAndMedianCTS::SegmentList;
using SegmentIndex = LunaCore::CTS::MeanAndMedianCTS::SegmentIndex;
using CTSInfo = LunaCore::CTS::MeanAndMedianCTS::CTSInfo;

BOOST_AUTO_TEST_CASE(check_cts)
{
    std::cout << "--== CHECK CTS ==--\n";

    LunaCore::CTS::MeanAndMedianCTS cts;

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_REQUIRE(leffile.good());
    
    std::ifstream leffile2("test/files/iit_stdcells_extra/fake_ties018.lef");
    BOOST_REQUIRE(leffile2.good());

    std::ifstream libfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    BOOST_REQUIRE(libfile.good());

    ChipDB::Design design;
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_REQUIRE(ChipDB::LEF::Reader::load(design, leffile2));
    BOOST_REQUIRE(ChipDB::Liberty::Reader::load(design, libfile));

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
    vp.canvasWidth  = 2000;
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
        if (segment.hasCell())
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
        if (segment.hasCell())
        {
            endNodes.push_back(index);            
        }
        index++;
    }

    // check that the number of end nodes we found
    // is equal to the number of sinks on the clk net
    auto clkNet = netlist->lookupNet("clk");
    BOOST_REQUIRE(clkNet.isValid());
    std::cout << "  End node count: " << endNodes.size() << "\n";
    std::cout << "  Net connects  : " << clkNet->numberOfConnections() << "\n";
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

    std::cout << "  Min : " << minL << "  max : " << maxL << "\n";
    std::cout << "  Min segs : " << minSegs << "  max segs : " << maxSegs << "\n";
    std::cout << "  Min segs index : " << minSegIndex << "  max segs index : " << maxSegIndex << "\n";

    // make sure the starting points are instances
    BOOST_CHECK(tree.value().at(minSegIndex).hasCell());
    BOOST_CHECK(tree.value().at(maxSegIndex).hasCell());

    // write out the min and max routes
    TinySVGPP::Canvas canvas2;
    canvas2.setSize(vp.canvasWidth, vp.canvasHeight);
    canvas2.fill("black").stroke("black").rect(0,0, vp.canvasWidth, vp.canvasHeight);

    int segIndex = minSegIndex;
    while (segIndex >= 0)
    {
        BOOST_REQUIRE(segIndex >= 0);
        BOOST_REQUIRE(segIndex < tree.value().size());

        auto const& seg = tree.value().at(segIndex);

        canvas2.stroke("green", 1.0).fill("green");
        canvas2.circle(vp.toWindow(toSVGPoint(seg.m_start)), 2);
        if (seg.hasCell())
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

    std::ofstream svgfile2("test/files/results/cts_minroute.svg");
    canvas2.toSVG(svgfile2);
    svgfile2.close();

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
        if (seg.hasCell())
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

    std::ofstream svgfile3("test/files/results/cts_maxroute.svg");
    canvas3.toSVG(svgfile3);
    svgfile3.close();

    // get the total capacitance of the clock network
    auto cap = cts.calcSegmentLoadCapacitance(0, tree.value(), *netlist);
    auto capPerNode = cap / static_cast<float>(endNodes.size());
    std::cout << "  Net capacitance    : " << cap << " F\n";
    std::cout << "  Average cap per ins: " << capPerNode << " F (expected = 0.0279235e-12)\n";

    BOOST_CHECK_CLOSE(capPerNode, 0.0279235e-12, 0.0001);    // this number comes from the DFFPOSX1 CLK pin.

    // insert buffers
    auto bufferCell = design.m_cellLib->lookupCell("CLKBUF2");
    BOOST_REQUIRE(bufferCell.isValid());

    auto bufferInput = bufferCell->lookupPin("A");
    BOOST_REQUIRE(bufferInput.isValid());

    auto bufferOutput = bufferCell->lookupPin("Y");
    BOOST_REQUIRE(bufferOutput.isValid());

    LunaCore::CTS::MeanAndMedianCTS::CTSInfo ctsinfo;

    ctsinfo.m_pinCapacitance = bufferInput->m_cap;
    ctsinfo.m_inputPinKey    = bufferInput.key();
    ctsinfo.m_outputPinKey   = bufferOutput.key();
    ctsinfo.m_bufferCell     = bufferCell.ptr();
    ctsinfo.m_maxCap         = 0.2e-12;     // 200 fF, no idea if this is realistic
    ctsinfo.m_clkNetKey      = clkNet.key();
    
    BOOST_CHECK(ctsinfo.m_inputPinKey != ChipDB::ObjectNotFound);
    BOOST_CHECK(ctsinfo.m_outputPinKey != ChipDB::ObjectNotFound);

    LunaCore::CTS::MeanAndMedianCTS::BufferResult bresult;
    bresult = cts.insertBuffers(tree.value(), 0, *netlist, ctsinfo);
    std::cout << "  Buffered net has " << bresult.m_totalCapacitance << " F at the input\n";

    // connect clk net to buffer
    for(auto const& sink : bresult.m_list)
    {
        // connect net to instance
        clkNet->addConnection(sink.m_instanceKey, sink.m_pinKey);

        // connect instance to net
        auto connInsKeyPtr = netlist->lookupInstance(sink.m_instanceKey);
        connInsKeyPtr->setPinNet(sink.m_pinKey, clkNet.key());
    }

    // write the verilog netlist
    std::ofstream ofile("test/files/results/cts.v");
    BOOST_REQUIRE(ofile.good());
    BOOST_CHECK(LunaCore::Verilog::Writer::write(ofile, mod.ptr()));
}

BOOST_AUTO_TEST_SUITE_END()