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

BOOST_AUTO_TEST_SUITE(LEFReaderTest)


BOOST_AUTO_TEST_CASE(can_read_lef)
{
    std::cout << "--== LEF READER ==--\n";

    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    BOOST_CHECK(design.m_techLib->getNumberOfLayers() > 0);
    BOOST_CHECK(design.m_techLib->getNumberOfSites() > 0);

    std::cout << "  Found " << design.m_cellLib->size() << " cells\n";
    BOOST_CHECK(design.m_cellLib->size() == 37);

    for(auto cellKeyObjPair : *design.m_cellLib)
    {
        std::cout << "  " << cellKeyObjPair->name() << "\n";
    }

    // check parameters of NANDX1 cell
    auto cellKeyObjPtr = design.m_cellLib->lookupCell("NAND2X1");
    BOOST_CHECK(cellKeyObjPtr.isValid());
    BOOST_CHECK(cellKeyObjPtr->m_pins.size() == 5);

    BOOST_CHECK(cellKeyObjPtr->m_size.m_x == 2400);
    BOOST_CHECK(cellKeyObjPtr->m_size.m_y == 10000);
    BOOST_CHECK(cellKeyObjPtr->m_offset.m_x == 0);
    BOOST_CHECK(cellKeyObjPtr->m_offset.m_y == 0);
    BOOST_CHECK(cellKeyObjPtr->m_site == "core");
    BOOST_CHECK((cellKeyObjPtr->m_symmetry.m_flags & ChipDB::SymmetryFlags::SYM_X) > 0);
    BOOST_CHECK((cellKeyObjPtr->m_symmetry.m_flags & ChipDB::SymmetryFlags::SYM_Y) > 0);
    BOOST_CHECK((cellKeyObjPtr->m_symmetry.m_flags & ChipDB::SymmetryFlags::SYM_R90) == 0);

    std::cout << "  NAND2X1 pins:\n";
    for(auto const pinInfo : cellKeyObjPtr->m_pins)
    {
        std::cout << "    " << pinInfo->name() << "\n";
    }

    auto pin = cellKeyObjPtr->lookupPin("A");
    BOOST_REQUIRE(pin.isValid());

    BOOST_CHECK(pin->m_iotype == ChipDB::IOType::INPUT);
    BOOST_CHECK(!pin->m_pinLayout.empty()); // check that the pin has geometry

    pin = cellKeyObjPtr->lookupPin("B");
    BOOST_CHECK(pin.isValid());
    BOOST_CHECK(pin->m_iotype == ChipDB::IOType::INPUT);
    BOOST_CHECK(!pin->m_pinLayout.empty()); // check that the pin has geometry

    pin = cellKeyObjPtr->lookupPin("Y");
    BOOST_CHECK(pin.isValid());
    BOOST_CHECK(pin->m_iotype == ChipDB::IOType::OUTPUT);
    BOOST_CHECK(!pin->m_pinLayout.empty()); // check that the pin has geometry

    // OAI22X1
    cellKeyObjPtr = design.m_cellLib->lookupCell("OAI22X1");
    BOOST_CHECK(cellKeyObjPtr.isValid());
    BOOST_CHECK(cellKeyObjPtr->m_pins.size() == 7);

    // check that the cell has obstructions
    BOOST_CHECK(cellKeyObjPtr->m_obstructions.size() != 0);
}

BOOST_AUTO_TEST_CASE(can_read_lef2)
{
    std::cout << "--== LEF READER NANGATE ==--\n";

    std::ifstream leffile("test/files/nangate/ocl.lef");
    BOOST_REQUIRE(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    BOOST_CHECK(design.m_techLib->getNumberOfLayers() > 0);
    BOOST_CHECK(design.m_techLib->getNumberOfSites() > 0);

    std::cout << "  Found " << design.m_cellLib->size() << " cells\n";
    BOOST_CHECK(design.m_cellLib->size() == 138);
}

BOOST_AUTO_TEST_CASE(can_read_techlef2)
{
    std::cout << "--== LEF READER NANGATE TECH ==--\n";

    std::ifstream leffile("test/files/nangate/ocl.tech.lef");
    BOOST_REQUIRE(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));

    std::cout << "  Found " << design.m_techLib->getNumberOfLayers() << " layers:\n";
    BOOST_CHECK(design.m_techLib->getNumberOfLayers() == 22);

    for(auto const layerKeyObjPair : design.m_techLib->layers())
    {
        std::cout << "    " << layerKeyObjPair->name() << "\n";
    }

    std::cout << "  Found " << design.m_techLib->getNumberOfSites() << " sites:\n";
    BOOST_CHECK(design.m_techLib->getNumberOfSites() == 1);
    for(auto const site : design.m_techLib->sites())
    {
        std::cout << "    " << site->name() << "\n";
    }

    // check site parameters
    BOOST_CHECK(design.m_techLib->sites().at(0)->name() == "FreePDK45_38x28_10R_NP_162NW_34O");
    BOOST_CHECK(design.m_techLib->sites().at(0)->m_symmetry.m_flags == ChipDB::SymmetryFlags::SYM_Y);
    BOOST_CHECK(design.m_techLib->sites().at(0)->m_class == ChipDB::SiteClass::CORE);
    BOOST_CHECK((design.m_techLib->sites().at(0)->m_size == ChipDB::Coord64{190,1400}));
}

BOOST_AUTO_TEST_CASE(can_read_lef3)
{
    std::cout << "--== LEF READER SKY130 TECH+CELL ==--\n";

    std::ifstream leffile("test/files/sky130/sky130_fd_sc_hd.tlef");
    BOOST_REQUIRE(leffile.good());

    std::ifstream leffile2("test/files/sky130/sky130_fd_sc_hd.lef");
    BOOST_REQUIRE(leffile2.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile2));

    std::cout << "  Found " << design.m_techLib->getNumberOfLayers() << " layers:\n";
    BOOST_CHECK(design.m_techLib->getNumberOfLayers() == 13);

    for(auto const layerKeyObjPair : design.m_techLib->layers())
    {
        std::cout << "    " << layerKeyObjPair->name() << "\n";
    }

    std::cout << "  Found " << design.m_techLib->getNumberOfSites() << " sites:\n";
    BOOST_CHECK(design.m_techLib->getNumberOfSites() == 2);
    for(auto const site : design.m_techLib->sites())
    {
        std::cout << "    " << site->name() << "\n";
    }

    // check site parameters
    BOOST_CHECK(design.m_techLib->sites().at(0)->name() == "unithd");
    BOOST_CHECK(design.m_techLib->sites().at(0)->m_symmetry.m_flags == ChipDB::SymmetryFlags::SYM_Y);
    BOOST_CHECK(design.m_techLib->sites().at(0)->m_class == ChipDB::SiteClass::CORE);
    BOOST_CHECK((design.m_techLib->sites().at(0)->m_size == ChipDB::Coord64{460, 2720}));

    std::cout << "  Found " << design.m_cellLib->size() << " cells\n";
    BOOST_CHECK(design.m_cellLib->size() == 441);
}

BOOST_AUTO_TEST_CASE(can_read_lef4)
{
    std::cout << "--== LEF READER IHP130 TECH+CELL ==--\n";

    std::ifstream leffile("test/files/ihp130/sg13g2_tech.lef");
    BOOST_REQUIRE(leffile.good());

    std::ifstream leffile2("test/files/ihp130/sg13g2_stdcell.lef");
    BOOST_REQUIRE(leffile2.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile));
    BOOST_CHECK(ChipDB::LEF::Reader::load(design, leffile2));

    std::cout << "  Found " << design.m_techLib->getNumberOfLayers() << " layers:\n";
    BOOST_CHECK(design.m_techLib->getNumberOfLayers() == 19);

    for(auto const layerKeyObjPair : design.m_techLib->layers())
    {
        std::cout << "    " << layerKeyObjPair->name() << "\n";
    }

    std::cout << "  Found " << design.m_techLib->getNumberOfSites() << " sites:\n";
    BOOST_CHECK(design.m_techLib->getNumberOfSites() == 1);
    for(auto const site : design.m_techLib->sites())
    {
        std::cout << "    " << site->name() << "\n";
    }

    // check site parameters
    BOOST_CHECK(design.m_techLib->sites().at(0)->name() == "CoreSite");
    BOOST_CHECK(design.m_techLib->sites().at(0)->m_symmetry.m_flags == ChipDB::SymmetryFlags::SYM_Y);
    BOOST_CHECK(design.m_techLib->sites().at(0)->m_class == ChipDB::SiteClass::CORE);
    BOOST_CHECK((design.m_techLib->sites().at(0)->m_size == ChipDB::Coord64{480, 3780}));

    std::cout << "  Found " << design.m_cellLib->size() << " cells\n";
    BOOST_CHECK(design.m_cellLib->size() == 82);

    for(auto cellKeyObjPair : *design.m_cellLib)
    {
        std::cout << "  " << cellKeyObjPair->name() << "\n";
    }

    std::cout << "== END IHP130 ==\n\n";
}

BOOST_AUTO_TEST_SUITE_END()
