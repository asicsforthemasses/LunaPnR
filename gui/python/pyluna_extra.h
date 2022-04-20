/*  LunaPnR Source Code
 
    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once

#include "lunacore.h"
#include "common/database.h"
#include "console/mmconsole.h"

namespace GUI
{

class Python : public Scripting::Python
{
public:
    Python(GUI::Database *db, GUI::MMConsole *console);
    virtual ~Python();

    bool preInitHook() override;
    bool postInitHook() override;

protected:
    GUI::Database   *m_db;
    GUI::MMConsole  *m_console;
};

};