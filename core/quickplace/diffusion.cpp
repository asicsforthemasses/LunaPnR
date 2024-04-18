#include "diffusion.hpp"
#include "common/common.h"

namespace LunaCore::QuickPlace
{

Diffusion::Diffusion(Database &db, ChipDB::Module &mod,
    const ChipDB::Rect64 &placementRect)
    : m_db(db), m_mod(mod), m_placementRect(placementRect)
{

}

bool Diffusion::init()
{
    auto siteName = m_db.m_design.m_floorplan->coreSiteName();
    auto siteKp = m_db.m_design.m_techLib->sites()[siteName];

    if (!siteKp.isValid())
    {
        Logging::logError("Could not find site %s\n", siteName.c_str());
        return false;
    }

    m_coreSiteSize = siteKp->m_size;

    calcAverageInstanceSize();

    if (m_averageSize.isNullSize())
    {
        return false;
    }

    // In case of 1M instances and cellsPerDimension = 5
    // a bin array will hold 1000000 / (cellsPerDimension*cellsPerDimension) =
    // 40k bins.
    const int cellsPerDimension = 5;
    ChipDB::Size64 binSize{
        m_averageSize.m_x * cellsPerDimension,
        m_averageSize.m_y * cellsPerDimension
        };

    Logging::logDebug("  bin size is %ld x %ld nm\n",
        binSize.m_x, binSize.m_y);

    m_bins.init(binSize, m_placementRect);

    initDensityMap();

    return true;
}

void Diffusion::initDensityMap()
{
    auto const& instances = m_mod.m_netlist->m_instances;
    for(auto insKp : instances)
    {
        assert(insKp.isValid());
        m_bins.addInstance(*insKp);
    }
}

void Diffusion::step(float dt)
{
    const float eps = 1e-6f;

    auto binCount = m_bins.getBinCount();

    // calculate the velocities in every bin
    for(int y=0; y<binCount.m_y; y++)
    {
        for(int x=0; x<binCount.m_x; x++)
        {
            auto &b = m_bins.at(x,y);

            auto const& d_left   = m_bins.at(x-1,y).m_density;
            auto const& d_right  = m_bins.at(x+1,y).m_density;
            auto const& d_top    = m_bins.at(x,y+1).m_density;
            auto const& d_bottom = m_bins.at(x,y-1).m_density;

            if ((d_right + d_left) > eps)
            {
                b.m_vx = -(d_right - d_left) / (d_right + d_left);
            }
            else
            {
                b.m_vx = 0.0f;
            }

            if ((d_top + d_bottom) > eps)
            {
                b.m_vy = -(d_top - d_bottom) / (d_top + d_bottom);
            }
            else
            {
                b.m_vy = 0.0f;
            }
        }
    }

    auto &instances = m_mod.m_netlist->m_instances;
    auto const binSize = m_bins.getBinSize();

    for(auto insKp : instances)
    {
        assert(insKp.isValid());
        if (!insKp->isFixed())
        {
            auto pos    = insKp->m_pos;
            auto bindex = m_bins.binIndex(pos);    // closest centre

            // find the fractional coordinates inside
            // the bin. we base the 2D linear interpolation on that
            // as well as which surrounding bins to use.
            pos -= m_placementRect.m_ll;    // compensate for bin area offset
            float frac_x = static_cast<float>(pos.m_x % binSize.m_x) / static_cast<float>(binSize.m_x);
            float frac_y = static_cast<float>(pos.m_y % binSize.m_y) / static_cast<float>(binSize.m_y);

            float ivx = 0.0f;   // interpolated velocity
            float ivy = 0.0f;

            auto &bin1 = m_bins.at(bindex.m_x, bindex.m_y);

            if (frac_x < 0.5f)
            {
                // interpolate with the bin to the left
                auto &bin2 = m_bins.at(bindex.m_x-1, bindex.m_y);

                frac_x += 0.5f;
                ivx = bin2.m_vx + frac_x*(bin1.m_vx-bin2.m_vx);
            }
            else
            {
                // interpolate with the bin to the right
                auto &bin2 = m_bins.at(bindex.m_x+1, bindex.m_y);

                frac_x -= 0.5f;
                ivx = bin1.m_vx + frac_x*(bin2.m_vx-bin1.m_vx);
            }

            if (frac_y < 0.5f)
            {
                // interpolate with the bin to the bottom
                auto &bin2 = m_bins.at(bindex.m_x, bindex.m_y-1);

                frac_y += 0.5f;
                ivy = bin2.m_vy + frac_y*(bin1.m_vy-bin2.m_vy);
            }
            else
            {
                // interpolate with the bin to the top
                auto &bin2 = m_bins.at(bindex.m_x, bindex.m_y+1);

                frac_y -= 0.5f;
                ivy = bin1.m_vy + frac_y*(bin2.m_vy-bin1.m_vy);
            }

        }
    }

    // update densities based upon diffusion time-step
    // note: fixed modules generate density
    // but the update formula does not account for this density
    // to be static.
    //
    // a solution might be to split each bin into
    // movable density and static density.
    for(int y=0; y<binCount.m_y; y++)
    {
        for(int x=0; x<binCount.m_x; x++)
        {
            auto &b = m_bins.at(x,y);

            auto const& d_left   = m_bins.at(x-1,y).m_density;
            auto const& d_right  = m_bins.at(x+1,y).m_density;
            auto const& d_top    = m_bins.at(x,y+1).m_density;
            auto const& d_bottom = m_bins.at(x,y-1).m_density;

            b.m_dnew  = (dt/2.0) * (d_right+d_left-2.0*b.m_density);
            b.m_dnew += (dt/2.0) * (d_top+d_bottom-2.0*b.m_density);
        }
    }

    for(auto &b : m_bins)
    {
        b.m_density = b.m_dnew;
    }
}

bool Diffusion::calcAverageInstanceSize()
{
    double m_xsum = 0.0;
    double m_ysum = 0.0;

    m_averageSize.m_x = 0;
    m_averageSize.m_y = 0;

    if (!m_mod.m_netlist)
    {
        Logging::logError("Module %s does not have a netlist\n", m_mod.name().c_str());
        m_averageSize = ChipDB::Size64{0,0};
        return false;
    }

    auto const& instances = m_mod.m_netlist->m_instances;
    std::size_t insCount = 0;
    for(auto const& instance : instances)
    {
        if (!instance->isFixed())
        {
            auto sz = instance->instanceSize();
            m_xsum += sz.m_x;
            m_ysum += sz.m_y;
            insCount++;
        }
    }

    if (insCount > 0)
    {
        m_averageSize.m_x = m_xsum / static_cast<double>(insCount);
        m_averageSize.m_y = m_ysum / static_cast<double>(insCount);
    }

    // Round up to nearest valid site position
    m_averageSize.m_x = roundUp(m_averageSize.m_x, m_coreSiteSize.m_x);
    m_averageSize.m_y = roundUp(m_averageSize.m_y, m_coreSiteSize.m_y);

    Logging::logDebug("  Average instance size: (%ld,%ld)\n",
        m_averageSize.m_x, m_averageSize.m_y);

    return true;
}

};
