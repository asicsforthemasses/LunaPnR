#pragma once
#include <iostream>
#include "database/database.h"

namespace LunaCore::GDS2
{

bool write(std::ostream &os, const Database &database);

};