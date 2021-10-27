#include "lunacore.h"

#include <sstream>
#include <fstream>
#include <random>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(PlacementBitmapTest)

BOOST_AUTO_TEST_CASE(check_placmentbitmap)
{
    std::cout << "--== CHECK PLACEMENT BITMAP ==--\n";

    LunaCore::DensityBitmap bm(255,255);

    std::default_random_engine generator;
    std::normal_distribution<double> distribution(128.0,20.0);

    for(uint32_t i=0; i<100000; i++)
    {
        auto x = static_cast<int64_t>(distribution(generator));
        auto y = static_cast<int64_t>(distribution(generator));
        bm.at(x,y) += 1.0f;
    }

    std::ofstream ofile("test/files/results/densitybitmap.pgm");

    BOOST_CHECK(ofile.good());

    if (ofile.good())
    {
        bm.writeToPGM(ofile);
    }
}

BOOST_AUTO_TEST_SUITE_END()