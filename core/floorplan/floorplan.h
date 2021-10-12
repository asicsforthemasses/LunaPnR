#pragma once

#include "common/dbtypes.h"
#include "common/namedstorage.h"
#include "region.h"

namespace ChipDB
{

class Floorplan
{
public:
    NamedStorage<Region*, true> m_regions;

protected:

};

};