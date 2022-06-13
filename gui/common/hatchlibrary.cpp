// SPDX-FileCopyrightText: 2021-2022 Niels Moseley, <n.a.moseley@moseleyinstruments.com>, et al.
//
// SPDX-License-Identifier: GPL-3.0-only

#include <iostream>
#include "hatchlibrary.h"
#include "../common/guihelpers.h"

using namespace GUI;

HatchLibrary::HatchLibrary()
{
    auto p = createPixmapFromString(
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "                
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "
        "*     *     *     *     *     "        
        ,30,16
    );
    
    if (p.has_value())
    {
        m_hatches.push_back(*p);
    } 

    p = createPixmapFromString(
        "*           *           "
        "                        "
        "      *           *     "
        "                        "
        "*           *           "
        "                        "
        "      *           *     "
        "                        "
        "*           *           "
        "                        "
        "      *           *     "
        "                        "
        "*           *           "
        "                        "
        "      *           *     "
        "                        "                        
        ,24,16
    );
    
    if (p.has_value())
    {
        m_hatches.push_back(*p);
    } 

    p = createPixmapFromString(
        "*   *   *   *   *   *   *   *   "
        " *   *   *   *   *   *   *   *  "
        "  *   *   *   *   *   *   *   * "
        "   *   *   *   *   *   *   *   *"
        "*   *   *   *   *   *   *   *   "
        " *   *   *   *   *   *   *   *  "
        "  *   *   *   *   *   *   *   * "
        "   *   *   *   *   *   *   *   *"
        "*   *   *   *   *   *   *   *   "
        " *   *   *   *   *   *   *   *  "
        "  *   *   *   *   *   *   *   * "
        "   *   *   *   *   *   *   *   *"
        "*   *   *   *   *   *   *   *   "
        " *   *   *   *   *   *   *   *  "
        "  *   *   *   *   *   *   *   * "
        "   *   *   *   *   *   *   *   *"                
        ,32,16
    );
    
    if (p.has_value())
    {
        m_hatches.push_back(*p);
    } 

    p = createPixmapFromString(
        "*   *   *   *   *   *   *   *   "
        "   *   *   *   *   *   *   *   *"
        "  *   *   *   *   *   *   *   * "
        " *   *   *   *   *   *   *   *  "
        "*   *   *   *   *   *   *   *   "
        "   *   *   *   *   *   *   *   *"
        "  *   *   *   *   *   *   *   * "
        " *   *   *   *   *   *   *   *  "
        "*   *   *   *   *   *   *   *   "
        "   *   *   *   *   *   *   *   *"
        "  *   *   *   *   *   *   *   * "
        " *   *   *   *   *   *   *   *  "
        "*   *   *   *   *   *   *   *   "
        "   *   *   *   *   *   *   *   *"
        "  *   *   *   *   *   *   *   * "
        " *   *   *   *   *   *   *   *  "
        ,32,16
    );

    if (p.has_value())
    {
        m_hatches.push_back(*p);
    }

    p = createPixmapFromString(
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        ,32,16
    );
    
    if (p.has_value())
    {
        m_hatches.push_back(*p);
    }   

    p = createPixmapFromString(
        "* * * * * * * * * * * * * * * * "
        "                                "
        "                                "
        "                                "
        "* * * * * * * * * * * * * * * * "
        "                                "
        "                                "
        "                                "
        "* * * * * * * * * * * * * * * * "
        "                                "
        "                                "
        "                                "
        "* * * * * * * * * * * * * * * * "
        "                                "
        "                                "
        "                                "                
        ,32,16
    );

    p = createPixmapFromString(
        "* * * * * * * * * * * * * * * * "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "        
        "* * * * * * * * * * * * * * * * "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "* * * * * * * * * * * * * * * * "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "* * * * * * * * * * * * * * * * "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "
        "*   *   *   *   *   *   *   *   "                        
        ,32,16
    );
    
    if (p.has_value())
    {
        m_hatches.push_back(*p);
    }

    p = createPixmapFromString(
        " ******    *******     ******            "
        "*      *   *      *   *                  "
        "*      *   *      *   *                  "
        "*      *   *******     ******            "
        "*      *   *      *          *           "
        "*      *   *      *          *           "
        " ******    *******     ******            "
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        "                                         "
        ,41,12
    );
    
    if (p.has_value())
    {
        m_hatches.push_back(*p);
    }
}
