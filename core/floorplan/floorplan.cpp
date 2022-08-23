// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "floorplan.h"

using namespace ChipDB;

void Floorplan::clear()
{
    m_regions.clear();
}

KeyObjPair<Region> Floorplan::createRegion(const std::string &name)
{
    auto optKeyObjPair = m_regions.add(std::make_shared<Region>(name));
    if (optKeyObjPair)
    {
        return optKeyObjPair.value();
    }
    
    return KeyObjPair<Region>();
}

std::shared_ptr<Region> Floorplan::lookupRegion(ChipDB::ObjectKey key) const
{
    return m_regions[key];
}

KeyObjPair<Region> Floorplan::lookupRegion(const std::string &name) const
{
    return m_regions[name];
}

bool Floorplan::removeRegion(const std::string &name)
{
    return m_regions.remove(name);
}

bool Floorplan::removeRegion(ChipDB::ObjectKey key)
{
    return m_regions.remove(key);
}

void Floorplan::addListener(INamedStorageListener *listener)
{
    m_regions.addListener(listener);
}

void Floorplan::removeListener(INamedStorageListener *listener)
{
    m_regions.removeListener(listener);
}

void Floorplan::contentsChanged() const
{
    m_regions.contentsChanged();
}
