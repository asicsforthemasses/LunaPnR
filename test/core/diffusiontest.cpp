// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <lunacore.h>
#include <tinysvgpp.h>
#include <tinytestpp.hpp>
#include <iomanip>
#include <random>
#include "../../core/quickplace/diffusion.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(DiffusionTest)

class DiffusionTester : public LunaCore::QuickPlace::Diffusion
{
public:
    DiffusionTester(LunaCore::Database &db,
        ChipDB::Module &mod,
        ChipDB::Rect64 placementRect)
        : LunaCore::QuickPlace::Diffusion(db, mod, placementRect)
        {
        }

    auto init(const float maxDensity)
    {
        return LunaCore::QuickPlace::Diffusion::init(maxDensity);
    }

    auto & binAt(const int x, const int y)
    {
        return LunaCore::QuickPlace::Diffusion::m_bins.at(x,y);
    }

    auto const& bins() const
    {
        return LunaCore::QuickPlace::Diffusion::m_bins;
    }

    constexpr auto getBinSize() const noexcept
    {
        return LunaCore::QuickPlace::Diffusion::m_bins.getBinSize();
    }
};

static void writePPM(const std::string &filename, ChipDB::Module &mod,
    const LunaCore::QuickPlace::Bin2D &bins)
{
    // write density bitmap
    LunaCore::PPM::Bitmap bm;
    auto bc = bins.getBinCount();
    bm.m_data.resize(bc.m_x * bc.m_y);
    bm.m_width = bc.m_x;
    bm.m_height = bc.m_y;

    for(int y=0; y<bc.m_y; y++)
    {
        for(int x=0; x<bc.m_x; x++)
        {
            uint8_t grey = static_cast<uint8_t>(255.0f*std::clamp(bins.at(x,y).m_density, 0.0f, 2.0f)/2.0f);
            LunaCore::PPM::RGB rgb{grey,grey,grey};
            bm.m_data.at(y*bc.m_x + x) = rgb;
        }
    }

    bm.m_data.at(0) = LunaCore::PPM::RGB{255,0,0};

    LunaCore::PPM::write(filename, bm);
}

static auto toSVGPt(const ChipDB::Coord64 &pos)
{
    return TinySVGPP::Point{
        static_cast<float>(pos.m_x),
        static_cast<float>(pos.m_y)
    };
}

struct FlipY
{
    FlipY(float canvasHeight) : m_canvasHeight(canvasHeight) {}

    TinySVGPP::Point operator()(const TinySVGPP::Point &p) const noexcept
    {
        return TinySVGPP::Point{p.x, m_canvasHeight - p.y - 1.0f};
    };

    float m_canvasHeight{0.0f};
};

static bool writeFloorplan(const std::string &filename,
    LunaCore::Database &db,
    const ChipDB::Module &mod,
    const LunaCore::QuickPlace::Bin2D &bins,
    const float maxDensity)
{
    auto const& floorplan = db.m_design.m_floorplan->coreRect();

    auto bc = bins.getBinCount();
    auto bs = bins.getBinSize();

    TinySVGPP::Canvas canvas;
    canvas.setSize(floorplan.getSize().m_x, floorplan.getSize().m_y);
    FlipY flipy(floorplan.getSize().m_y);

    std::ofstream svgfile(filename);
    if (!svgfile.is_open())
    {
        return false;
    }

    canvas.fill("black").rect(TinySVGPP::Point{0,0},
        toSVGPt(floorplan.getSize()));

    // draw bins
    canvas.fill("white", false).stroke("white");
    float unitLength = static_cast<float>(std::min(bs.m_x, bs.m_y));
    for(int y=0; y<bc.m_y; y++)
    {
        for(int x=0; x<bc.m_x; x++)
        {
            // draw bin
            auto const binLL = flipy(toSVGPt(ChipDB::Coord64{x*bs.m_x, y*bs.m_y}));
            auto const binUR = flipy(toSVGPt(ChipDB::Coord64{x*bs.m_x + bs.m_x, y*bs.m_y + bs.m_y}));

            if (bins.at(x,y).m_density > maxDensity)
            {
                canvas.stroke("red");
            }
            else
            {
                canvas.stroke("white");
            }
            canvas.opacity(0.25f).rect(binLL, binUR);
        }
    }

    auto const& instances = mod.m_netlist->m_instances;

    canvas.fill("white").opacity(0.5f);   // make rects semi-transparent
    for(auto insKp : instances)
    {
        if (!insKp.isValid()) continue;

        auto pos = insKp->m_pos;
        auto sz  = insKp->instanceSize();

        canvas.rect(
                flipy(toSVGPt(pos)),
                flipy(toSVGPt(pos + sz))
            );
    }

    canvas.toSVG(svgfile);
    return true;
}

static bool writeVelocityVectors(const std::string &filename,
    LunaCore::Database &db,
    const LunaCore::QuickPlace::Bin2D &bins)
{
    // write velocity vectors of bins to svg
    auto bc = bins.getBinCount();
    auto bs = bins.getBinSize();

    const int cwidth = bc.m_x * bs.m_x;
    const int cheight = bc.m_y * bs.m_y;

    TinySVGPP::Canvas canvas;
    canvas.setSize(cwidth, cheight);
    FlipY flipy(cheight);

    std::ofstream svgfile(filename);
    if (!svgfile.is_open())
    {
        return false;
    }

    canvas.fill("black").rect(TinySVGPP::Point{0,0},
        TinySVGPP::Point{static_cast<float>(cwidth), static_cast<float>(cheight)});

    canvas.fill("white", false).stroke("white");
    float unitLength = static_cast<float>(std::min(bs.m_x, bs.m_y));
    for(int y=0; y<bc.m_y; y++)
    {
        for(int x=0; x<bc.m_x; x++)
        {
            // draw bin
            auto const binLL = toSVGPt(ChipDB::Coord64{x*bs.m_x, y*bs.m_y});
            auto const binUR = toSVGPt(ChipDB::Coord64{x*bs.m_x + bs.m_x, y*bs.m_y + bs.m_y});
            canvas.opacity(0.25f).rect(binLL, binUR);

            // draw the velocity vector

            auto const& bin = bins.at(x,y);
            float mag2 = bin.m_vx*bin.m_vx + bin.m_vy*bin.m_vy;

            if (mag2 > 0.001f)
            {
                auto binCenter = TinySVGPP::Point{(binLL.x + binUR.x)/2.0f, (binLL.y + binUR.y)/2.0f};
                auto vecStart = binCenter;
                auto vecEnd   = vecStart + TinySVGPP::Point{
                    bins.at(x,y).m_vx * unitLength,
                    bins.at(x,y).m_vy * unitLength};

                canvas.opacity(0.75f).line(flipy(vecStart), flipy(vecEnd));
                canvas.circle(flipy(binCenter), unitLength/2.0f);

                canvas.opacity(0.25f).circle(flipy(vecEnd), 3);
            }
        }
    }

    canvas.toSVG(svgfile);
    return true;
}

BOOST_AUTO_TEST_CASE(diffusiontest, * boost::unit_test::tolerance(0.00001))
{
    std::cout << "--== DIFFUSION TEST ==--\n";

    auto ll = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::DEBUG);

    LunaCore::Database db;
    ChipDB::Rect64 placementRect{{0,0}, {1000,1000}};

    // 3 dummy cell types
    auto cell1Kp = db.m_design.m_cellLib->createCell("cell1");
    cell1Kp->m_size = ChipDB::Coord64{10,10};   // area 100

    auto cell2Kp = db.m_design.m_cellLib->createCell("cell2");
    cell2Kp->m_size = ChipDB::Coord64{30,10};   // area 300

    auto cell3Kp = db.m_design.m_cellLib->createCell("cell3");
    cell3Kp->m_size = ChipDB::Coord64{100,10};  // area 1000

    auto topKp = db.m_design.m_moduleLib->createModule("top");

    auto ins1 = std::make_shared<ChipDB::Instance>("ins1", ChipDB::InstanceType::CELL, cell1Kp.ptr());
    ins1->m_pos = ChipDB::Coord64{0,0};
    topKp->addInstance(ins1);

    // create enough of a "floorplan" to make the diffusor happy..
    auto siteKp = db.m_design.m_techLib->createSiteInfo("core");
    siteKp->m_size = ChipDB::Coord64{10,10};    // smallest valid core cell size

    db.m_design.m_floorplan->setCoreSiteName("core");
    db.m_design.m_floorplan->setCoreSize(placementRect.getSize());

    // init diffusor
    auto diff = std::make_unique<DiffusionTester>(db, *topKp, placementRect);
    const float maxDensity = 0.75f;
    BOOST_CHECK(diff->init(maxDensity));

    // bins are 50x50 = 2500 nm^2
    // cell1 is 10x10 = 100 nm^2
    // so expected density in bin(0,0) = 100 / 2500

    const float expectedDensity = 0.04f;
    BOOST_CHECK(diff->binAt(0,0).m_density == expectedDensity);
    BOOST_CHECK(diff->binAt(1,0).m_density == 0.0);
    BOOST_CHECK(diff->binAt(0,1).m_density == 0.0);
    Logging::logInfo("  Bin(0,0) density is %f\n", diff->binAt(0,0).m_density);

    // add more instances to overfill the bins
    auto binSize = diff->getBinSize();
    const ChipDB::Coord64 offset(10*binSize.m_x, 10*binSize.m_y);
    ins1->m_pos += offset; // offset to the middle
    for(int i=0; i<40; i++)
    {
        std::stringstream ss;
        ss << "ins" << i+2;
        auto ins = std::make_shared<ChipDB::Instance>(ss.str(), ChipDB::InstanceType::CELL, cell1Kp.ptr());
        ins->m_pos = offset + ChipDB::Coord64{2*i, 2*i}; // offset to the middle
        topKp->addInstance(ins);
    }

    BOOST_CHECK(diff->init(maxDensity));

    // check that the new instances are not just in bin(0,0)
    BOOST_CHECK(diff->binAt(10,10).m_density > 0.0f);
    BOOST_CHECK(diff->binAt(11,10).m_density > 0.0f);
    BOOST_CHECK(diff->binAt(10,11).m_density > 0.0f);

    for(int iter=0; iter<10; iter++)
    {
        std::stringstream ss;
        ss << "_" << std::setfill('0') << std::setw(2);
        ss << iter;

        writePPM("test/files/results/diff" + ss.str() + ".ppm", *topKp, diff->bins());
        BOOST_REQUIRE(writeFloorplan("test/files/results/diff_fp"  + ss.str() + ".svg", db, *topKp, diff->bins(), maxDensity));
        BOOST_REQUIRE(writeVelocityVectors("test/files/results/diff_vec" + ss.str() + ".svg", db, diff->bins()));

        diff->step(0.2f);
    }

    Logging::setLogLevel(ll);
}

BOOST_AUTO_TEST_CASE(randomdiffusiontest, * boost::unit_test::tolerance(0.00001))
{
    std::cout << "--== RANDOM DIFFUSION TEST ==--\n";

    auto ll = Logging::getLogLevel();
    Logging::setLogLevel(Logging::LogType::DEBUG);

    LunaCore::Database db;
    ChipDB::Rect64 placementRect{{0,0}, {1000,1000}};

    // 3 dummy cell types
    auto cell1Kp = db.m_design.m_cellLib->createCell("cell1");
    cell1Kp->m_size = ChipDB::Coord64{10,10};   // area 100
    cell1Kp->m_area = static_cast<double>(cell1Kp->m_size.m_x * cell1Kp->m_size.m_y) * 1.0e-6;

    auto cell2Kp = db.m_design.m_cellLib->createCell("cell2");
    cell2Kp->m_size = ChipDB::Coord64{30,10};   // area 300
    cell2Kp->m_area = static_cast<double>(cell2Kp->m_size.m_x * cell2Kp->m_size.m_y) * 1.0e-6;

    auto cell3Kp = db.m_design.m_cellLib->createCell("cell3");
    cell3Kp->m_size = ChipDB::Coord64{100,10};  // area 1000
    cell3Kp->m_area = static_cast<double>(cell3Kp->m_size.m_x * cell3Kp->m_size.m_y) * 1.0e-6;

    auto topKp = db.m_design.m_moduleLib->createModule("top");

    // create enough of a "floorplan" to make the diffusor happy..
    auto siteKp = db.m_design.m_techLib->createSiteInfo("core");
    siteKp->m_size = ChipDB::Coord64{10,10};    // smallest valid core cell size

    db.m_design.m_floorplan->setCoreSiteName("core");
    db.m_design.m_floorplan->setCoreSize(placementRect.getSize());

    // init diffusor
    auto diff = std::make_unique<DiffusionTester>(db, *topKp, placementRect);
    const float maxDensity = 0.75f;

    // add more instances to overfill the bins
    auto const placementCenter = placementRect.center();
    const ChipDB::Coord64 offset(placementCenter.m_x, placementCenter.m_y);

    std::random_device rd{};
    std::mt19937 gen{rd()};
    std::normal_distribution d{0.0f, 50.0f}; // 50.0f standard deviation
    std::uniform_real_distribution d2{0.0f, 30.0f}; // 0 .. 3

    auto randomPos = [&d, &gen]
        {
            return ChipDB::Coord64(static_cast<ChipDB::CoordType>(d(gen)), static_cast<ChipDB::CoordType>(d(gen)));
        };

    auto randomCell = [&d2, &gen, cell1Kp, cell2Kp, cell3Kp]
        {
            auto id = std::min(static_cast<int>(std::round(d2(gen))), 2);
            switch(id)
            {
            default:    // fallthrough
            case 0:
                return cell1Kp.ptr();
            case 1:
                return cell1Kp.ptr();
            case 2:
                return cell2Kp.ptr();
            }
        };

    double insArea = 0.0f;
    for(int i=0; i<1000; i++)
    {
        std::stringstream ss;
        ss << "ins" << i;
        auto ins = std::make_shared<ChipDB::Instance>(ss.str(), ChipDB::InstanceType::CELL, randomCell());
        ins->m_pos = offset + randomPos(); // offset to the middle
        topKp->addInstance(ins);

        insArea += ins->getArea();
    }

    auto const psz = placementRect.getSize();
    double layoutArea = static_cast<double>(psz.m_x)*psz.m_y / 1e6f;

    Logging::logInfo("Instance area: %f um^2\n", insArea);
    Logging::logInfo("Layout area  : %f um^2\n", layoutArea);

    auto initResult = diff->init(maxDensity);
    BOOST_CHECK(initResult.has_value());

    std::size_t overflowCount = initResult.value();

    const std::size_t maxIter = 500;
    std::size_t iter = 0;

    BOOST_REQUIRE(writeFloorplan("test/files/results/diff_rnd_fp_initial.svg", db, *topKp, diff->bins(), maxDensity));

    TinyTest::ProfilingTimer timer;
    timer.start();
    while((overflowCount > 0) && (iter < maxIter))
    {
        std::stringstream ss;
        ss << "_" << std::setfill('0') << std::setw(2);
        ss << iter;

        if (iter < 200)
        {
            BOOST_REQUIRE(writeFloorplan("test/files/results/diff_rnd_fp"  + ss.str() + ".svg", db, *topKp, diff->bins(), maxDensity));
            BOOST_REQUIRE(writeVelocityVectors("test/files/results/diff_rnd_vec" + ss.str() + ".svg", db, diff->bins()));
        }

        if ((iter % 5) == 4)
        {
            overflowCount = diff->renewDensities();
        }

        diff->step(0.2f);
        iter++;
    }
    auto ms = timer.stop();

    Logging::logInfo("Diffusion placer took %f ms abd %lu iterations\n", ms, iter);
    BOOST_CHECK(iter != maxIter);

    BOOST_REQUIRE(writeFloorplan("test/files/results/diff_rnd_fp_final.svg", db, *topKp, diff->bins(), maxDensity));

    Logging::setLogLevel(ll);
}

BOOST_AUTO_TEST_SUITE_END()
