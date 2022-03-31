#pragma once
#include <string>
#include "design/design.h"
namespace Scripting
{

class Python
{
public:
    Python(ChipDB::Design *design);
    virtual ~Python();

    bool executeScript(const std::string &code);

protected:
    ChipDB::Design *m_design;
};

};