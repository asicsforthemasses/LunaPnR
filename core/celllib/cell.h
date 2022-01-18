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

    std::string     m_name; ///< name of the cell
    PinInfoList     m_pins; ///< pin information

    double          m_area;         ///< area in um^2
    double          m_leakagePower; ///< in Watts
    Coord64         m_size;         ///< size in nm
    Coord64         m_offset;       ///< placement offset in nm

    SymmetryFlags   m_symmetry;
    CellClass       m_class;
    CellSubclass    m_subclass;
    std::string     m_site;

    /** create a pin, or return an already existing pin with the name */
    PinInfo*    createPin(const std::string &name);

    /** returns nullptr for non-existing pins */
    PinInfo*        lookupPin(const std::string &name);

    /** returns nullptr for non-existing pins */
    const PinInfo*  lookupPin(const std::string &name) const;

    /** returns nullptr for non-existing pins */
    PinInfo*        lookupPin(ssize_t index);

    /** returns nullptr for non-existing pins */
    const PinInfo*  lookupPin(ssize_t index) const;

    ssize_t     lookupPinIndex(const std::string &name) const;

    virtual bool isModule() const
    {
        return false;
    };

    std::unordered_map<std::string /* layer name */, GeometryObjects> m_obstructions;
};

}; // namespace
