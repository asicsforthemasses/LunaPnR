#pragma once
#include <vector>
#include <optional>
#include <cmath>
#include <memory>
#include "database/database.h"

namespace LunaCore::QuickPlace
{

class Bin2D
{
public:
    struct BinInfo
    {
        float m_density{0.0};   ///< density of bin
        float m_dnew{0.0};      ///< new density of bin
        float m_vx{0.0};        ///< cell x velocity in the middle of the bin
        float m_vy{0.0};        ///< cell y velocity in the middle of the bin
    };

    struct BinCount
    {
        int m_x{0};
        int m_y{0};
    };

    Bin2D() = default;

    void init(const ChipDB::Size64 &binSize, const ChipDB::Rect64 &extents)
    {
        m_binSize = binSize;
        m_extents = extents;

        m_sz = m_extents.getSize();

        m_binCount.m_x = static_cast<int>(std::ceil(m_sz.m_x / static_cast<float>(binSize.m_x)));
        m_binCount.m_y = static_cast<int>(std::ceil(m_sz.m_y / static_cast<float>(binSize.m_y)));

        // sanity?
        if (m_binCount.m_x == 0) m_binCount.m_x++;
        if (m_binCount.m_y == 0) m_binCount.m_y++;

        m_binArea = static_cast<float>(binSize.m_x * binSize.m_y);

        m_bins.clear();
        m_bins.resize(m_binCount.m_x*m_binCount.m_y);

        Logging::logDebug("  bin count is %ld x %ld\n", m_binCount.m_x, m_binCount.m_y);
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

    constexpr BinInfo& at(const int x, const int y) noexcept
    {
        return bin(x,y);
    }

    constexpr const BinInfo& at(const int x, const int y) const noexcept
    {
        return bin(x,y);
    }

    [[nodiscard]] constexpr BinCount getBinCount() const noexcept
    {
        return m_binCount;
    }

    [[nodiscard]] auto begin() noexcept
    {
        return m_bins.begin();
    }

    [[nodiscard]] auto end() noexcept
    {
        return m_bins.end();
    }

    [[nodiscard]] constexpr auto getBinSize() const noexcept
    {
        return m_binSize;
    }

    void addInstance(const ChipDB::Instance &ins)
    {
        updateBins(ins);
    }

    constexpr void setBoundaryDensity(const float density) noexcept
    {
        m_boundaryDensity  = density;
        m_dummy.m_density  = m_boundaryDensity;
        m_dummy2.m_density = m_boundaryDensity;
    }

protected:

    [[nodiscard]] constexpr BinInfo& bin(const int xIndex,  const int yIndex) noexcept
    {
        if ((xIndex < 0) || (yIndex < 0) || (xIndex >= m_binCount.m_x) || (yIndex >= m_binCount.m_y))
        {
            m_dummy.m_density = m_boundaryDensity;
            m_dummy.m_vx = 0.0f;
            m_dummy.m_vy = 0.0f;
            return m_dummy;
        }

        return m_bins.at(yIndex*m_binCount.m_x + xIndex);
    }

    [[nodiscard]] constexpr const BinInfo& bin(const int xIndex,  const int yIndex) const noexcept
    {
        if ((xIndex < 0) || (yIndex < 0) || (xIndex >= m_binCount.m_x) || (yIndex >= m_binCount.m_y))
        {
            return m_dummy2;
        }

        return m_bins.at(yIndex*m_binCount.m_x + xIndex);
    }

    /** Update the bins an instance occupies.
    */
    void updateBins(const ChipDB::Instance &ins)
    {
        assert(m_binArea > 0.0f);

        ChipDB::Rect64 insRect{ins.m_pos, ins.m_pos + ins.instanceSize()};

        auto const llBinIndex = binIndex(insRect.m_ll);
        auto const urBinIndex = binIndex(insRect.m_ur);

        for(auto yBinIndex = llBinIndex.m_y; yBinIndex <= urBinIndex.m_y; yBinIndex++)
        {
            for(auto xBinIndex = llBinIndex.m_x; xBinIndex <= urBinIndex.m_x; xBinIndex++)
            {
                ChipDB::Coord64 binLL   // lower left position of bin
                {
                    m_extents.m_ll.m_x + xBinIndex * m_binSize.m_x,
                    m_extents.m_ll.m_y + yBinIndex * m_binSize.m_y
                };

                ChipDB::Rect64 binRect{binLL, binLL+m_binSize};
                auto overlapFactor = ChipDB::areaOverlap(binRect, insRect) / m_binArea;

                bin(xBinIndex, yBinIndex).m_density += overlapFactor;
            }
        }
    }

    ChipDB::Size64 m_sz;        ///< total size of the binned area
    ChipDB::Rect64 m_extents;   ///< extents of the binned area
    ChipDB::Size64 m_binSize;   ///< size of one bin in nm
    float m_binArea{0.0};       ///< area of one bin in nm^2

    BinCount m_binCount;        ///< number of bins in the horizontal and vertical direction
    BinInfo m_dummy2{0};        ///< dummy density entry for out-of-bound bin access
    BinInfo m_dummy{0};         ///< dummy density entry for out-of-bound bin access

    float m_boundaryDensity{0.0f};  ///< density returned for bins beyond the boundary

    std::vector<BinInfo> m_bins;
};

/** diffusion placer */
class Diffusion
{
public:

    Diffusion(Database &db,
        ChipDB::Module &mod,
        const ChipDB::Rect64 &placementRect);

    /** returns the number of bins that are above the maxDensity threshold */
    std::optional<std::size_t> init(const float maxDensity);

    /** update placement of cells based on the bin densities
        @param[in] dt time step (should be smaller than 1.0?)
    */
    void step(float dt);

    /** update the density map based on the actual instances,
        not the predicted densities.
        call this every 5 or so iterations.
    */
    std::size_t renewDensities();

protected:
    bool calcAverageInstanceSize();
    std::size_t initDensityMap();   ///<

    LunaCore::Database &m_db;       ///< database
    ChipDB::Module &m_mod;          ///< the module to place
    ChipDB::Rect64 m_placementRect; ///< placement area/rectangle
    ChipDB::Size64 m_averageSize;   ///< average instance size
    ChipDB::Size64 m_coreSiteSize;  ///< core site minimum cell size / minimum placement grid

    float m_maxDensity{0.75f};      ///< maximum cell density per bin

    Bin2D m_bins;
};

};