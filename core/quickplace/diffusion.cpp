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

    Logging::logInfo("  bin size is %ld x %ld nm\n",
        binSize.m_x, binSize.m_y);

    return true;
}

void Diffusion::step(float dt)
{
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

    // FIXME: round it to the nearest size specified
    // by the site.

    m_averageSize.m_x = roundUp(m_averageSize.m_x, m_coreSiteSize.m_x);
    m_averageSize.m_y = roundUp(m_averageSize.m_y, m_coreSiteSize.m_y);

    Logging::logDebug("  Average instance size: (%ld,%ld)\n",
        m_averageSize.m_x, m_averageSize.m_y);

    return true;
}

};
