#pragma once

#include "lunacore.h"
#include "common/database.h"

namespace GUI
{

class Python : public Scripting::Python
{
public:
    Python(GUI::Database *db);
    virtual ~Python();

    bool preInitHook() override;
    bool postInitHook() override;

protected:
    GUI::Database *m_db;
};

};