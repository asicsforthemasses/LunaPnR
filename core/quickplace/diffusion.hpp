#pragma once
#include <vector>
#include <optional>
#include <cmath>
#include "database/database.h"

namespace LunaCore::QuickPlace
{

class Bin2D
{
    using Density = float;  ///< type for bin density information

public:
    Bin2D(const ChipDB::Size64 &binSize, const ChipDB::Rect64 &extents)
        : m_binSize(binSize),
          m_extents(extents)
    {
        m_sz = m_extents.getSize();

        m_xBinCount = static_cast<int>(std::ceil(m_sz.m_x / static_cast<float>(binSize.m_x)));
        m_yBinCount = static_cast<int>(std::ceil(m_sz.m_y / static_cast<float>(binSize.m_y)));

        m_binArea = static_cast<float>(binSize.m_x * binSize.m_y);

        m_bins.resize(m_xBinCount*m_yBinCount, 0);

        Logging::logDebug("Bin2D : bin cound is %ld x %ld\n", m_xBinCount, m_yBinCount);
    }

    struct BinIndex
    {
        int m_x{0};
        int m_y{0};

        [[nodiscard]] constexpr bool isValid(const ChipDB::Size64 &totalSize) const noexcept
        {
            if ((m_x < 0) || (m_y < 0)) return false;
            if ((m_x >= totalSize.m_x) || (m_y >= totalSize.m_y)) return false;

            return true;
        }
    };

    [[nodiscard]] constexpr BinIndex binIndex(const ChipDB::Coord64 &pos) const noexcept
    {
        BinIndex bindex;
        auto refPos = pos - m_extents.m_ll;
        bindex.m_x = (refPos.m_x / m_binSize.m_x);
        bindex.m_y = (refPos.m_y / m_binSize.m_y);
        return bindex;
    }

    /** Update the bins an instance occupies.
    */
    void updateBins(const ChipDB::Instance &ins)
    {
        ChipDB::Rect64 insRect{ins.m_pos, ins.m_pos + ins.instanceSize()};

        auto const llBinIndex = binIndex(insRect.m_ll);
        auto const urBinIndex = binIndex(insRect.m_ur);

        for(auto yBinIndex = llBinIndex.m_y; yBinIndex <= urBinIndex.m_y; yBinIndex++)
        {
            for(auto xBinIndex = llBinIndex.m_x; xBinIndex <= urBinIndex.m_x; xBinIndex++)
            {
                ChipDB::Rect64 binRect;
                auto overlap = ChipDB::areaOverlap(binRect, insRect);
            }
        }

        auto const center = insRect.center();
        auto const bindex = binIndex(center);
        if (bindex.isValid(m_sz))
        {
            //auto o = insRect.intersect();
        }
    }

protected:
    ChipDB::Size64 m_sz;
    ChipDB::Rect64 m_extents;
    ChipDB::Size64 m_binSize;
    float m_binArea{0.0};

    int m_xBinCount{0};
    int m_yBinCount{0};

    /** return the area of overlap in nm^2 */
    float areaOverlap(const ChipDB::Rect64 &r1, const ChipDB::Rect64 &r2)
    {
        auto xo = std::min(r1.right(), r2.right()) - std::max(r1.left(), r2.left());
        auto yo = std::min(r1.top(), r2.top()) - std::max(r1.bottom(), r2.bottom());

        if ((xo > 0) && (yo > 0))
        {
            return static_cast<float>(xo)*static_cast<float>(yo);
        }

        return 0.0f;
    }

    std::vector<Density> m_bins;
};

/** diffusion placer */
class Diffusion
{
public:
    Diffusion(Database &db, ChipDB::Module &mod,
        const ChipDB::Rect64 &placementRect);

    bool init();
    void step(float dt);

protected:
    bool calcAverageInstanceSize();

    LunaCore::Database &m_db;       ///< database
    ChipDB::Module &m_mod;          ///< the module to place
    ChipDB::Rect64 m_placementRect; ///< placement area/rectangle
    ChipDB::Size64 m_averageSize;   ///< average instance size
    ChipDB::Size64 m_coreSiteSize;  ///< core site placement size
};

};