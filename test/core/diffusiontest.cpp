// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <lunacore.h>
#include <tinysvgpp.h>
#include <iomanip>
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

    bool init()
    {
        return LunaCore::QuickPlace::Diffusion::init();
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
    ChipDB::Module &mod)
{
    auto const& floorplan = db.m_design.m_floorplan->coreRect();

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
            auto const binLL = flipy(toSVGPt(ChipDB::Coord64{x*bs.m_x, y*bs.m_y}));
            auto const binUR = flipy(toSVGPt(ChipDB::Coord64{x*bs.m_x + bs.m_x, y*bs.m_y + bs.m_y}));
            canvas.opacity(0.25f).rect(binLL, binUR);

            // draw the velocity vector

            auto const& bin = bins.at(x,y);
            float mag2 = bin.m_vx*bin.m_vx + bin.m_vy*bin.m_vy;

            if (mag2 > 0.01f)
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
    cell2Kp->m_size = ChipDB::Coord64{100,10};  // area 1000

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
    BOOST_CHECK(diff->init());

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

    BOOST_CHECK(diff->init());

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
        BOOST_REQUIRE(writeFloorplan("test/files/results/diff_fp"  + ss.str() + ".svg", db, *topKp));
        BOOST_REQUIRE(writeVelocityVectors("test/files/results/diff_vec" + ss.str() + ".svg", db, diff->bins()));

        diff->step(0.2f);
    }

    Logging::setLogLevel(ll);
}


BOOST_AUTO_TEST_SUITE_END()
