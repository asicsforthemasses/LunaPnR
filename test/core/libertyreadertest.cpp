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
    BOOST_CHECK(libertyfile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::Liberty::Reader::load(&design, libertyfile));

    std::cout << "  Found " << design.m_cellLib.size() << " cells:\n";
    BOOST_CHECK(design.m_cellLib.size() == 32);

    for(auto cell : design.m_cellLib)
    {
        std::cout << "  " << cell->m_name << "\n";
    }

    // check parameters of NANDX1 cell
    auto cell = design.m_cellLib.lookup("NAND2X1");
    BOOST_CHECK(cell != nullptr);
    BOOST_CHECK(cell->m_pins.size() == 3);

    std::cout << "  NAND2X1 pins:\n";
    for(auto pin : cell->m_pins)
    {
        std::cout << "    " << pin.m_name << "  max cap " << pin.m_maxCap << "  function: " << pin.m_function << "\n";
    }    
}

BOOST_AUTO_TEST_SUITE_END()
