// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include "common/dbtypes.h"
#include "common/geometry.h"
#include "common/namedstorage.h"
#include "pin.h"

namespace ChipDB
{

class Cell
{
public:
    Cell() : m_area(0), m_leakagePower(0), m_class(CellClass::CORE), m_subclass(CellSubclass::NONE)
    {
    }

    Cell(const std::string &name) : m_name(name), m_area(0), 
        m_leakagePower(0), m_class(CellClass::CORE), m_subclass(CellSubclass::NONE) 
    {
    }

    std::string name() const noexcept
    {
        return m_name;
    }

    size_t getNumberOfPins() const noexcept
    {
        return m_pins.size();
    }

    PinInfoList     m_pins;         ///< pin information

    double          m_area;         ///< area in um^2
    double          m_leakagePower; ///< in Watts
    Coord64         m_size;         ///< size in nm
    Coord64         m_offset;       ///< placement offset in nm

    SymmetryFlags   m_symmetry;
    CellClass       m_class;
    CellSubclass    m_subclass;
    std::string     m_site;

    /** create a named pin and return the object and key */
    KeyObjPair<PinInfo> createPin(const std::string &name);

    /** returns nullptr for non-existing pins */
    KeyObjPair<PinInfo> lookupPin(const std::string &name);

    /** returns nullptr for non-existing pins */
    KeyObjPair<PinInfo> lookupPin(const std::string &name) const;

    /** returns nullptr for non-existing pins */
    std::shared_ptr<PinInfo> lookupPin(ObjectKey key);

    /** returns nullptr for non-existing pins */
    std::shared_ptr<PinInfo> lookupPin(ObjectKey key) const;

    virtual bool isModule() const
    {
        return false;
    };

    std::unordered_map<std::string /* layer name */, GeometryObjects> m_obstructions;

protected:
    std::string     m_name;         ///< name of the cell
};

}; // namespace
