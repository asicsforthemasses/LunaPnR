// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#pragma once

#include <optional>
#include "dbtypes.h"
#include "namedstorage.h"
#include "region.h"

namespace ChipDB
{

class Floorplan
{
public:

    /** add an already created region to the floorplan. returns an invalid KeyObjPair if a region
     *  of the same name already exists */
    KeyObjPair<Region> addRegion(std::shared_ptr<Region> regionPtr);

    /** create a region with the specified name and add it to the floorplan. returns an invalid KeyObjPair if a region
     *  of the same name already exists */
    KeyObjPair<Region> createRegion(const std::string &name, const std::string &site);

    std::shared_ptr<Region> lookupRegion(ChipDB::ObjectKey key) const;
    KeyObjPair<Region> lookupRegion(const std::string &name) const;
    [[nodiscard]] bool removeRegion(const std::string &name);
    [[nodiscard]] bool removeRegion(ChipDB::ObjectKey key);

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

    [[nodiscard]] size_t regionCount() const noexcept
    {
        return m_regions.size();
    }

protected:
    NamedStorage<Region> m_regions;
};

};
