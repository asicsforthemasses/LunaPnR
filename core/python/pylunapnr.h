#pragma once
#include <string>

namespace Scripting
{

class Python
{
public:
    Python();
    virtual ~Python();

    bool executeScript(const std::string &code);
};

};