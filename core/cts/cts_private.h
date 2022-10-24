#pragma once
#include "common/dbtypes.h"

namespace LunaCore::CTS
{

/** node holding the instance key and the instance position */
struct Node
{
    ChipDB::ObjectKey m_insKey;
    ChipDB::Coord64   m_pos;
};

};
