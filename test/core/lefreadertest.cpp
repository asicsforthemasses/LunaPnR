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
    BOOST_CHECK(design.m_cellLib.size() == 33);

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
        std::cout << "    " << pin.m_name << "\n";
    }

    auto pin = cell->lookupPin("A");
    BOOST_CHECK(pin != nullptr);
    BOOST_CHECK(pin->m_iotype == ChipDB::IO_INPUT);

    pin = cell->lookupPin("B");
    BOOST_CHECK(pin != nullptr);
    BOOST_CHECK(pin->m_iotype == ChipDB::IO_INPUT);

    pin = cell->lookupPin("Y");
    BOOST_CHECK(pin != nullptr);
    BOOST_CHECK(pin->m_iotype == ChipDB::IO_OUTPUT);    
}

BOOST_AUTO_TEST_SUITE_END()
