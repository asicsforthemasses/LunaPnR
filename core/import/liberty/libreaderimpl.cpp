// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only


#include <array>
#include "common/logging.h"
#include "libreaderimpl.h"

using namespace ChipDB::Liberty;

ReaderImpl::ReaderImpl(Design &design)
    : m_design(design), m_curCell(nullptr), m_curPin(nullptr)
{
    // default units as a fallback
    m_capacitanceUnit = 1e-12; // 1pf
    m_leakagePowerUnit = 1e-9; // 1nW
}

void ReaderImpl::parseLeakagePowerUnit(const std::string &value)
{
    // according to the 2017 Liberty specification,
    // valid units are: 1mW, 100uW, 10uW, 1uW, 100nW, 10nW, 1nW, 100pW, 10pW, and 1pW

    static const std::array<std::string, 10> text =
        {"1mW", "100uW", "10uW", "1uw", "100nW", "10nW", "1nW", "100pW", "10pW", "1pW"};

    static const std::array<double, 10> unit =
        {1e-3, 100e-6, 10e-6, 1e-6, 100e-9, 10e-9, 1e-9, 100e-12, 10e-12, 1e-12};

    for (size_t idx = 0; idx < text.size(); idx++)
    {
        if (text[idx] == value)
        {
            m_leakagePowerUnit = unit[idx];
            return;
        }
    }

    Logging::doLog(Logging::LogType::WARNING, "Power leakage unit %s in Liberty file is malformed.\n", value.c_str());
}

void ReaderImpl::parseCapacitanceUnit(const std::string &value, const std::string &unit)
{
    if (unit == "ff")
    {
        m_capacitanceUnit = stod(value) * 1e-15;
    }
    else if (unit == "pf")
    {
        m_capacitanceUnit = stod(value) * 1e-12;
    }
    else
    {
        Logging::doLog(Logging::LogType::WARNING, "Capacitance unit %s in Liberty file is malformed.\n", unit.c_str());
        m_capacitanceUnit = 1e-12;
    }
}

void ReaderImpl::onGroup(const std::string &group)
{
    m_groupStack.push(GT_NONE);
}

void ReaderImpl::onGroup(const std::string &group, const std::string &name)
{
    if (group == "library")
    {
        m_groupStack.push(GT_LIBRARY);

#if 0
        // FIXME: what if we want to load more than one library
        //        and keep them separate?

        m_lib = m_design->getLibraryByName("default");
        if (m_lib == nullptr)
        {
            // library with this name not found.
            // so make one.
            m_lib = m_design->createLibrary("default");
        }
#endif
    }
    else if (group == "cell")
    {
        m_groupStack.push(GT_CELL);
        auto cellKeyObjPair = m_design.m_cellLib->createCell(name);
        m_curCell = cellKeyObjPair.ptr();
    }
    else if (group == "pin")
    {
        // find pin on cell
        if (m_curCell)
        {
            auto pinInfoKeyObjPair  = m_curCell->createPin(name);

            pinInfoKeyObjPair->m_maxCap = 0;
            pinInfoKeyObjPair->m_maxFanOut = 0;

            m_curPin = pinInfoKeyObjPair.ptr();
        }
        else
        {
            Logging::doLog(Logging::LogType::ERROR, "DBLibertyReader: cell ptr should exist but doesn't.");
        }

        m_groupStack.push(GT_PIN);
    }
    else
    {
        m_groupStack.push(GT_NONE);
    }
}

void ReaderImpl::onSimpleAttribute(const std::string &name, const std::string &value)
{
    switch (m_groupStack.top())
    {
    case GT_LIBRARY:
        if (name == "leakage_power_unit")
        {
            parseLeakagePowerUnit(value);
        }
        break;
    case GT_PIN:
    {
        if (!m_curPin)
        {
            Logging::doLog(Logging::LogType::ERROR, "onSimpleAttribute pin = nullptr\n");
            return;
        }

        if (name == "capacitance")
        {
            m_curPin->m_cap = std::stof(value) * m_capacitanceUnit;
        }
        else if (name == "max_capacitance")
        {
            m_curPin->m_maxCap = std::stof(value) * m_capacitanceUnit;
        }
        else if (name == "max_fanout")
        {
            m_curPin->m_maxFanOut = std::stof(value);
        }
        else if (name == "function")
        {
            m_curPin->m_function = value;
        }
        else if (name == "clock")
        {
            if (value == "true")
            {
                m_curPin->m_clock = true;
            }
            else
            {
                m_curPin->m_clock = false;
            }
        }
        else if (name == "direction")
        {
            // Only store direction if there is no
            // other previous information about the pin.
            // The reason for this is that the LEF
            // file has more elaborate information
            // on the pin io type.
            if (m_curPin->m_iotype == ChipDB::IOType::UNKNOWN)
            {
                if (value == "input")
                    m_curPin->m_iotype = ChipDB::IOType::INPUT;
                else if (value == "output")
                    m_curPin->m_iotype = ChipDB::IOType::OUTPUT;
            }
        }
        else if (name == "three_state")
        {
            if ((m_curPin->m_iotype == ChipDB::IOType::UNKNOWN) ||
                (m_curPin->m_iotype == ChipDB::IOType::OUTPUT))
            {
                m_curPin->m_iotype = ChipDB::IOType::OUTPUT_TRI;
                m_curPin->m_tristateFunction = value;
            }
        }
        break;
    }
    case GT_CELL:
    {
        if (m_curCell == nullptr)
        {
            Logging::doLog(Logging::LogType::ERROR, "onSimpleAttribute cell = nullptr\n");
            return;
        }

        /*

            is_decap_cell : true | false;  
            is_filler_cell : true | false;  
            is_tap_cell : true | false;

            */

        if (name == "cell_leakage_power")
        {
            m_curCell->m_leakagePower = std::stof(value) * m_leakagePowerUnit;
        }
        else if (name == "area")
        {
            // Only store area if the area is 0
            // The reason for this is that the LEF
            // file takes presedence.
            if (m_curCell->m_area < 1e-20)
            {
                m_curCell->m_area = std::stod(value);
            }
        }
    }
    break;
    default:
        break;
    };
}

void ReaderImpl::onComplexAttribute(const std::string &attrname, const std::vector<std::string> &list)
{
}

void ReaderImpl::onEndGroup()
{
    switch (m_groupStack.top())
    {
    case GT_LIBRARY:
        break;
    case GT_CELL:
        break;
    case GT_PIN:
        break;
    default:;
    };
}

void ReaderImpl::onEndParse()
{
}
