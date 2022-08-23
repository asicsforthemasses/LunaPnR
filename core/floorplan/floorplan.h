// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#pragma once

#include <optional>
#include "common/dbtypes.h"
#include "common/namedstorage.h"
#include "region.h"

namespace ChipDB
{

class Floorplan
{
public:

    KeyObjPair<Region> createRegion(const std::string &name);
    std::shared_ptr<Region> lookupRegion(ChipDB::ObjectKey key) const;
    KeyObjPair<Region> lookupRegion(const std::string &name) const;
    bool removeRegion(const std::string &name);
    bool removeRegion(ChipDB::ObjectKey key);

    void clear();

    void addListener(INamedStorageListener *listener);
    void removeListener(INamedStorageListener *listener);

    /** notify all listeners that the floorplan has changed */
    void contentsChanged() const;

    auto begin() const
    {
        return m_regions.begin();
    }

    auto end() const
    {
        return m_regions.end();
    }

    size_t regionCount() const noexcept
    {
        return m_regions.size();
    }

protected:
    NamedStorage<Region> m_regions;
};

};
