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

    void clear();

    void addListener(INamedStorageListener *listener);
    void removeListener(INamedStorageListener *listener);

protected:
    NamedStorage<Region> m_regions;
};

};
