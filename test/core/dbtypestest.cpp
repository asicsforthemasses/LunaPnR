#include "lunacore.h"

#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DBTypesTest)

BOOST_AUTO_TEST_CASE(check_Coord64)
{
    std::cout << "--== CHECK COORD64 ==--\n";
    
    ChipDB::Coord64 p1;
    BOOST_CHECK((p1.m_x == 0) && (p1.m_y == 0));

    ChipDB::Coord64 p2 = {20,70};
    BOOST_CHECK((p2.m_x == 20) && (p2.m_y == 70));

    p1 = ChipDB::Coord64(10,20);
    BOOST_CHECK((p1.m_x == 10) && (p1.m_y == 20));

    p1 = ChipDB::Coord64(-10,-20);
    BOOST_CHECK((p1.m_x == -10) && (p1.m_y == -20));

    p1 += ChipDB::Coord64(-10,-20);
    BOOST_CHECK((p1.m_x == -20) && (p1.m_y == -40));

    p1 -= ChipDB::Coord64(-10,-20);
    BOOST_CHECK((p1.m_x == -10) && (p1.m_y == -20));

    p1 = ChipDB::Coord64(-10,-20) + ChipDB::Coord64(-10,-20);
    BOOST_CHECK((p1.m_x == -20) && (p1.m_y == -40));

    p1 = ChipDB::Coord64(-10,-20) - ChipDB::Coord64(-10,-20);
    BOOST_CHECK((p1.m_x == 0) && (p1.m_y == 0));

    // check if streams compile
    p1 = ChipDB::Coord64(-10,-20);
    std::cout << "p1 = " << p1 << "\n";

}

BOOST_AUTO_TEST_CASE(check_Rect64)
{
    std::cout << "--== CHECK RECT64 ==--\n";
    
    // check if streams compile
    ChipDB::Rect64 r1({0,0},{10,10});

    std::cout << "r1 = " << r1 << "\n";

    r1.setSize({100,150});
    BOOST_CHECK((r1.m_ll.m_x == 0) && (r1.m_ll.m_y == 0));
    BOOST_CHECK((r1.m_ur.m_x == 100) && (r1.m_ur.m_y == 150));

    BOOST_CHECK((r1.getLL() == ChipDB::Coord64{0,0}));
    BOOST_CHECK((r1.getUR() == ChipDB::Coord64{100,150}));
    BOOST_CHECK((r1.getLR() == ChipDB::Coord64{100,0}));
    BOOST_CHECK((r1.getUL() == ChipDB::Coord64{0,150}));

    r1.moveTo({200,200});
    BOOST_CHECK((r1.m_ll.m_x == 200) && (r1.m_ll.m_y == 200));
    BOOST_CHECK((r1.m_ur.m_x == 300) && (r1.m_ur.m_y == 350));
    
    r1.moveBy({50,50});
    BOOST_CHECK((r1.m_ll.m_x == 250) && (r1.m_ll.m_y == 250));
    BOOST_CHECK((r1.m_ur.m_x == 350) && (r1.m_ur.m_y == 400));

    auto center = r1.center();
    BOOST_CHECK((center.m_x == 250+50) && (center.m_y == 250+75));

    auto w = r1.width();

    BOOST_CHECK(r1.width() == 100);
    BOOST_CHECK(r1.height() == 150);

    // test margins on rectangle
    auto r1_reference = r1;
    
    ChipDB::Margins64 margins{10,10,20,20}; // top, bottom, left, right
    r1.contract(margins);

    BOOST_CHECK(r1.left() == (r1_reference.left()+20));
    BOOST_CHECK(r1.bottom() == (r1_reference.bottom()+10));
    BOOST_CHECK(r1.right() == (r1_reference.right()-20));
    BOOST_CHECK(r1.top() == (r1_reference.top()-10));

    r1.expand(margins);
    BOOST_CHECK(r1.left() == r1_reference.left());
    BOOST_CHECK(r1.bottom() == r1_reference.bottom());
    BOOST_CHECK(r1.right() == r1_reference.right());
    BOOST_CHECK(r1.top() == r1_reference.top());
}


BOOST_AUTO_TEST_SUITE_END()