// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(LibertyReaderTest)


BOOST_AUTO_TEST_CASE(can_read_Liberty)
{
    std::cout << "--== LIBERTY READER ==--\n";
    
    std::ifstream libertyfile("test/files/iit_stdcells/lib/tsmc018/signalstorm/iit018_stdcells.lib");
    BOOST_REQUIRE(libertyfile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::cout << "  Found " << design.m_cellLib->size() << " cells:\n";
    BOOST_CHECK(design.m_cellLib->size() == 36);

    for(auto cell : *design.m_cellLib)
    {
        std::cout << "  " << cell->name() << "\n";
    }

    // check parameters of NANDX1 cell
    auto cellKeyObjPair = design.m_cellLib->lookupCell("NAND2X1");
    BOOST_REQUIRE(cellKeyObjPair.isValid());
    BOOST_CHECK(cellKeyObjPair->m_pins.size() == 3);

    std::cout << "  NAND2X1 pins:\n";
    for(auto pinInfoPtr : cellKeyObjPair->m_pins)
    {
        std::cout << "    " << pinInfoPtr->name() << "  max cap " << pinInfoPtr->m_maxCap << "  function: " << pinInfoPtr->m_function << "\n";
    }   

    // check pins of a complex cell, like FAX1
    auto fax1 = design.m_cellLib->lookupCell("FAX1");
    BOOST_REQUIRE(fax1.isValid());
    std::cout << "  FAX1 pin count: " << fax1->m_pins.size() << "\n";
    BOOST_CHECK(fax1->m_pins.size() == 5);

    for(auto pinInfoPtr : fax1->m_pins)
    {
        std::cout << "    " << pinInfoPtr->name() << "  max cap " << pinInfoPtr->m_maxCap << "  function: " << pinInfoPtr->m_function << "\n";
    } 

    auto ysPin = fax1->lookupPin("YS");
    BOOST_REQUIRE(ysPin.isValid());
    BOOST_CHECK(ysPin->isOutput());

}

BOOST_AUTO_TEST_CASE(can_read_Liberty2)
{
    std::cout << "--== LIBERTY READER NANGATE==--\n";
    
    std::ifstream libertyfile("test/files/nangate/lpocl_functional.lib");
    BOOST_REQUIRE(libertyfile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::cout << "  Found " << design.m_cellLib->size() << " cells:\n";
    BOOST_CHECK(design.m_cellLib->size() == 40);

#if 0
    for(auto cell : design.m_cellLib)
    {
        std::cout << "  " << cell->m_name << "\n";
    }
#endif
}

BOOST_AUTO_TEST_CASE(can_read_Liberty3)
{
    std::cout << "--== LIBERTY READER NANGATE 2==--\n";
    
    std::ifstream libertyfile("test/files/nangate/ocl_functional.lib");
    BOOST_REQUIRE(libertyfile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::cout << "  Found " << design.m_cellLib->size() << " cells:\n";
    BOOST_CHECK(design.m_cellLib->size() == 138);

#if 0
    for(auto cell : design.m_cellLib)
    {
        std::cout << "  " << cell->m_name << "\n";
    }
#endif
}

BOOST_AUTO_TEST_CASE(can_read_Liberty4)
{
    std::cout << "--== LIBERTY READER SKY130 ==--\n";
    
    std::ifstream libertyfile("test/files/sky130/sky130_fd_sc_hd__tt_025C_1v80.lib");
    BOOST_REQUIRE(libertyfile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::cout << "  Found " << design.m_cellLib->size() << " cells:\n";
    BOOST_CHECK(design.m_cellLib->size() == 138);
}

BOOST_AUTO_TEST_CASE(can_read_Liberty5)
{
    std::cout << "--== LIBERTY READER SKY130 2 ==--\n";
    
    std::ifstream libertyfile("test/files/sky130/sky130_fd_sc_hd__tt_100C_1v80.lib");
    BOOST_REQUIRE(libertyfile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::Liberty::Reader::load(design, libertyfile));

    std::cout << "  Found " << design.m_cellLib->size() << " cells:\n";
    BOOST_CHECK(design.m_cellLib->size() == 138);
}

BOOST_AUTO_TEST_SUITE_END()
