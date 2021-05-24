
#include "design.h"

using namespace ChipDB;

Module* Design::createModule(const std::string &name)
{
    auto newModule = m_moduleLib.createModule(name);
    return newModule;
}

