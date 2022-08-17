/*
    LunaPnR Source Code

    SPDX-License-Identifier: GPL-3.0-only
    SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#pragma once

#include <vector>
#include <string>
#include <iostream>

namespace GUI
{

/** LunaPnR project information, used by the project browser.
    This should hold only project persistent data
*/
struct ProjectSetup
{
    std::vector<std::string> m_lefFiles;
    std::vector<std::string> m_libFiles;
    std::vector<std::string> m_verilogFiles;
    std::vector<std::string> m_timingConstraintFiles;

    std::string m_openSTALocation{"/usr/local/bin/sta"};

    bool readFromJSON(std::istream &is);
    bool writeToJSON(std::ostream &os) const;
};

};
