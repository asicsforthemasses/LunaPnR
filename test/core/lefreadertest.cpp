#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(LEFgReaderTest)


BOOST_AUTO_TEST_CASE(can_read_lef)
{
    std::cout << "--== LEF READER ==--\n";
    
    std::ifstream leffile("test/files/iit_stdcells/lib/tsmc018/lib/iit018_stdcells.lef");
    BOOST_CHECK(leffile.good());

    ChipDB::Design design;
    BOOST_CHECK(ChipDB::LEF::Reader::load(&design, leffile));

    std::cout << "  Found " << design.m_cellLib.size() << " cells\n";
    BOOST_CHECK(design.m_cellLib.size() == 33);
}

BOOST_AUTO_TEST_SUITE_END()
