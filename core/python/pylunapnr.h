#pragma once
#include <type_traits>
#include <functional>
#include <string>
#include "design/design.h"

namespace Scripting
{

class Python
{
public:
    Python(ChipDB::Design *design);
    virtual ~Python();

    void init();

    virtual bool preInitHook() { return true; };
    virtual bool postInitHook() { return true; };

    bool executeScript(const std::string &code);

    void setConsoleRedirect(std::function<void(const char *, ssize_t)> stdoutFunc,
        std::function<void(const char *, ssize_t)> stderrFunc);

protected:
    ChipDB::Design *m_design;
};

};