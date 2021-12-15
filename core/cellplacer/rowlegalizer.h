#pragma once

#include "design/design.h"

namespace LunaCore
{

class RowLegalizer
{
public:

    struct Cluster
    {
        ChipDB::CoordType m_start;  ///< start of interval in nm
        ChipDB::CoordType m_width;  ///< width of interval in nm
    };

    struct RowInfo
    {
        ChipDB::Rect64 m_rect;
        constexpr double getAvailableArea() const
        {
            return m_rect.width() * m_rect.height();
        }

        constexpr double getOccupiedArea() const
        {
            return 0; // FIXME
        }

        /** if no overlap, it returns -1, else the cluster index */
        ssize_t hasOverlap(ChipDB::CoordType xpos, ChipDB::CoordType width) const;

        /** create a cluster and insert it into the sorted cluster vector 
        */
        void createCluster(ChipDB::CoordType xpos, ChipDB::CoordType width);

        double addAndCollapseCluster(ChipDB::CoordType xpos, ChipDB::CoordType width);

        std::vector<Cluster> m_clusters;
    };

    bool legalize(const ChipDB::Design &design, ChipDB::Module &module, ChipDB::Region &region);
    bool checkAllInstancesPlaced(const ChipDB::Module &module);

    enum class InsertMode
    {
        Trial,
        Place
    };

    double insertInstance(ChipDB::InstanceBase *ins, RowInfo &row, InsertMode mode);
};

};