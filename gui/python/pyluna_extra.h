// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#ifdef USE_PYTHON

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

#endif
