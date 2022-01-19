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
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::cout << "  Found " << design.m_cellLib.size() << " cells\n";
    BOOST_CHECK(design.m_cellLib.size() == 34);

    for(auto cell : design.m_cellLib)
    {
        std::cout << "  " << cell->m_name << "\n";
    }    

    // check parameters of NANDX1 cell
    auto cell = design.m_cellLib.lookup("NAND2X1");
    BOOST_CHECK(cell != nullptr);
    BOOST_CHECK(cell->m_pins.size() == 5);

    BOOST_CHECK(cell->m_size.m_x == 2400);
    BOOST_CHECK(cell->m_size.m_y == 10000);
    BOOST_CHECK(cell->m_offset.m_x == 0);
    BOOST_CHECK(cell->m_offset.m_y == 0);
    BOOST_CHECK(cell->m_site == "core");
    BOOST_CHECK((cell->m_symmetry.m_flags & ChipDB::SymmetryFlags::SYM_X) > 0);
    BOOST_CHECK((cell->m_symmetry.m_flags & ChipDB::SymmetryFlags::SYM_Y) > 0);
    BOOST_CHECK((cell->m_symmetry.m_flags & ChipDB::SymmetryFlags::SYM_R90) == 0);

    std::cout << "  NAND2X1 pins:\n";
    for(auto pin : cell->m_pins)
    {
        std::cout << "    " << pin->m_name << "\n";
    }

    auto pin = cell->lookupPin("A");
    BOOST_CHECK(pin != nullptr);
    BOOST_CHECK(pin->m_iotype == ChipDB::IOType::INPUT);
    BOOST_CHECK(!pin->m_pinLayout.empty()); // check that the pin has geometry

    pin = cell->lookupPin("B");
    BOOST_CHECK(pin != nullptr);
    BOOST_CHECK(pin->m_iotype == ChipDB::IOType::INPUT);
    BOOST_CHECK(!pin->m_pinLayout.empty()); // check that the pin has geometry

    pin = cell->lookupPin("Y");
    BOOST_CHECK(pin != nullptr);
    BOOST_CHECK(pin->m_iotype == ChipDB::IOType::OUTPUT);    
    BOOST_CHECK(!pin->m_pinLayout.empty()); // check that the pin has geometry

    // OAI22X1
    cell = design.m_cellLib.lookup("OAI22X1");
    BOOST_CHECK(cell != nullptr);
    BOOST_CHECK(cell->m_pins.size() == 7);

    // check that the cell has obstructions
    BOOST_CHECK(cell->m_obstructions.size() != 0);
}

BOOST_AUTO_TEST_CASE(can_read_lef2)
{
    std::cout << "--== LEF READER NANGATE ==--\n";
    
    std::ifstream leffile("test/files/nangate/ocl.lef");
    BOOST_CHECK(leffile.good());

    if (!leffile.good())
    {
        std::cout << " skipping test because LEF fould could not be loaded\n";
        return;
    }

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::cout << "  Found " << design.m_cellLib.size() << " cells\n";
    BOOST_CHECK(design.m_cellLib.size() == 135);
}

BOOST_AUTO_TEST_CASE(can_read_techlef2)
{
    std::cout << "--== LEF READER NANGATE TECH ==--\n";
    
    std::ifstream leffile("test/files/nangate/ocl.tech.lef");
    BOOST_CHECK(leffile.good());

    if (!leffile.good())
    {
        std::cout << " skipping test because LEF fould could not be loaded\n";
        return;
    }

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::cout << "  Found " << design.m_techLib.getNumberOfLayers() << " layers:\n";
    BOOST_CHECK(design.m_techLib.getNumberOfLayers() == 22);

    for(auto const layerKeyObjPair : design.m_techLib.layers())
    {
        std::cout << "    " << layerKeyObjPair->name() << "\n";
    }

    std::cout << "  Found " << design.m_techLib.getNumberOfSites() << " sites:\n";
    BOOST_CHECK(design.m_techLib.getNumberOfSites() == 1);
    for(auto const site : design.m_techLib.sites())
    {
        std::cout << "    " << site->name() << "\n";
    }

    // check site parameters
    BOOST_CHECK(design.m_techLib.sites().at(0)->name() == "FreePDK45_38x28_10R_NP_162NW_34O");
    BOOST_CHECK(design.m_techLib.sites().at(0)->m_symmetry.m_flags == ChipDB::SymmetryFlags::SYM_Y);
    BOOST_CHECK(design.m_techLib.sites().at(0)->m_class == ChipDB::SC_CORE);
    BOOST_CHECK((design.m_techLib.sites().at(0)->m_size == ChipDB::Coord64{190,1400}));

}

BOOST_AUTO_TEST_SUITE_END()
