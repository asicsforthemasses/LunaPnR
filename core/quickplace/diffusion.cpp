#include "diffusion.hpp"
#include "common/common.h"

namespace LunaCore::QuickPlace
{

// Note: need to convert to floating point instance position
// so that instances do share coordinates less often.
// Another technique is to slightly randomize the positions, by 1nm
// We probably need an intelligent end placer to fix this..

Diffusion::Diffusion(Database &db, ChipDB::Module &mod,
    const ChipDB::Rect64 &placementRect)
    : m_db(db), m_mod(mod), m_placementRect(placementRect)
{

}

std::optional<std::size_t> Diffusion::init(const float maxDensity)
{
    m_maxDensity = maxDensity;

    auto siteName = m_db.m_design.m_floorplan->coreSiteName();
    auto siteKp = m_db.m_design.m_techLib->sites()[siteName];

    if (!siteKp.isValid())
    {
        Logging::logError("Could not find site %s\n", siteName.c_str());
        return std::nullopt;
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

    // make sure we have at least 10 bins in either direction
    float xbinCount = m_placementRect.getSize().m_x / binSize.m_x;
    float ybinCount = m_placementRect.getSize().m_y / binSize.m_y;

    if (xbinCount < 10.0f)
    {
        binSize.m_x = m_placementRect.getSize().m_x / 10;
        Logging::logWarning("Diffusion x bin count < 10: reducing the bin size.\n");
    }

    if (ybinCount < 10.0f)
    {
        binSize.m_y = m_placementRect.getSize().m_y / 10;
        Logging::logWarning("Diffusion y bin count < 10: reducing the bin size.\n");
    }

    Logging::logDebug("  bin size is %ld x %ld nm\n",
        binSize.m_x, binSize.m_y);

    m_bins.init(binSize, m_placementRect);

    return initDensityMap();
}

std::size_t Diffusion::initDensityMap()
{
    return renewDensities();
}

std::size_t Diffusion::renewDensities()
{
    std::size_t overflowCount = 0;

    for(auto &b : m_bins)
    {
        b.m_density = 0.0f;
    }

    auto const& instances = m_mod.m_netlist->m_instances;
    for(auto insKp : instances)
    {
        assert(insKp.isValid());
        m_bins.addInstance(*insKp);
    }

    // fill bins that are below max density
    // so that the average density of the
    // binned area equals maxDensity
    // this avoids superfluous spreading

    double areaLessThanDMax = 0.0;
    double areaOverDMax = 0.0;
    const double binArea_um2 = (m_bins.getBinSize().m_x*m_bins.getBinSize().m_y) * 1e-6;   // um^2

    assert(binArea_um2 > 0.0);

    float maxBinDensity = 0.0f;
    for(auto &b : m_bins)
    {
        maxBinDensity = std::max(maxBinDensity, b.m_density);
        if (b.m_density >= m_maxDensity)
        {
            overflowCount++;
            areaOverDMax += binArea_um2;
        }
        else
        {
            areaLessThanDMax += binArea_um2;
        }
    }

    assert(areaLessThanDMax > 0.0);

    const double ratio = areaOverDMax / areaLessThanDMax;
    Logging::logDebug("  max bin density        : %f\n", maxBinDensity);
    Logging::logDebug("  bin overflow count     : %lu\n", overflowCount);
    Logging::logDebug("  difussion density ratio: %f\n", ratio);

    for(auto &b : m_bins)
    {
        if (b.m_density < m_maxDensity)
        {
            b.m_density = m_maxDensity - (m_maxDensity - b.m_density)*ratio;
        }
    }

    m_bins.setBoundaryDensity(m_maxDensity - m_maxDensity*ratio);

    return overflowCount;
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

            assert(b.m_density >= 0.0f);

            if (b.m_density > eps)
            {
                b.m_vx = -0.5f*(d_right - d_left) / b.m_density;
            }
            else
            {
                b.m_vx = 0.0f;
            }

            if (b.m_density > eps)
            {
                b.m_vy = -0.5f*(d_top - d_bottom) / b.m_density;
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

            // use the center of the instance
            auto isz = insKp->instanceSize();
            pos += ChipDB::Coord64(isz.m_x/2, isz.m_y/2);

            float frac_x = static_cast<float>(pos.m_x % binSize.m_x) / static_cast<float>(binSize.m_x);
            float frac_y = static_cast<float>(pos.m_y % binSize.m_y) / static_cast<float>(binSize.m_y);

            // find the 4 closest cells to the instance
            float alpha = frac_x;
            float beta  = frac_y;
            if (frac_x <= 0.5f)
            {
                bindex.m_x--;
                alpha += 0.5f;
            }
            else
            {
                alpha -= 0.5f;
            }

            if (frac_y <= 0.5f)
            {
                bindex.m_y--;
                beta += 0.5f;
            }
            else
            {
                beta -= 0.5f;
            }

            auto const& bin1 = m_bins.at(bindex.m_x, bindex.m_y);
            float ivx = bin1.m_vx;
            ivx += alpha*(m_bins.at(bindex.m_x+1, bindex.m_y).m_vx-bin1.m_vx);
            ivx += beta*(m_bins.at(bindex.m_x, bindex.m_y+1).m_vx-bin1.m_vx);
            ivx += alpha*beta*(bin1.m_vx + m_bins.at(bindex.m_x+1, bindex.m_y+1).m_vx
                - m_bins.at(bindex.m_x+1, bindex.m_y).m_vx
                - m_bins.at(bindex.m_x, bindex.m_y+1).m_vx
                );

            float ivy = bin1.m_vy;
            ivy += alpha*(m_bins.at(bindex.m_x+1, bindex.m_y).m_vy-bin1.m_vy);
            ivy += beta*(m_bins.at(bindex.m_x, bindex.m_y+1).m_vy-bin1.m_vy);
            ivy += alpha*beta*(bin1.m_vy + m_bins.at(bindex.m_x+1, bindex.m_y+1).m_vy
                - m_bins.at(bindex.m_x+1, bindex.m_y).m_vy
                - m_bins.at(bindex.m_x, bindex.m_y+1).m_vy
                );

            // move the instance based on the calculate velocity
            // note: we need to multiply by the bin size because
            // that is the unit used by the density map..
            insKp->m_pos = ChipDB::Coord64(
                static_cast<ChipDB::CoordType>(insKp->m_pos.m_x + dt*ivx*binSize.m_x),
                static_cast<ChipDB::CoordType>(insKp->m_pos.m_y + dt*ivy*binSize.m_y)
            );
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

            b.m_dnew = b.m_density;
            b.m_dnew += (dt/2.0) * (d_right+d_left-2.0f*b.m_density);
            b.m_dnew += (dt/2.0) * (d_top+d_bottom-2.0f*b.m_density);
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
