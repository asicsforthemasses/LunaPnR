#pragma once
#include <string>
#include <unordered_map>

namespace ChipDB
{

using PropertyValue = std::string;
using PropertyName  = std::string;
using Properties    = std::unordered_map<PropertyName, PropertyValue>;

};
