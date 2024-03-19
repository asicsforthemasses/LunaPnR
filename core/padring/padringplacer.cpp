// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <stdexcept>
#include "padringplacer.hpp"

namespace LunaCore::Padring
{

class PadringConfigReader : public ConfigReader
{
public:
    PadringConfigReader(std::istream &is, Database &db, Padring &padring)
        : ConfigReader(is), m_db{db}, m_padring(padring)
    {
        m_padring.clear();
    }

    void onCorner(
        const std::string &instance,
        const std::string &location) override
    {
        auto modulePtr = m_db.m_design.getTopModule();
        if (!modulePtr)
        {
            throw std::runtime_error("Top module not set");
        }

        auto insKp = modulePtr->m_netlist->lookupInstance(instance);
        if (!insKp.isValid())
        {
            std::stringstream ss;
            ss << "  Cannot find instance " << instance << " for corner cell\n";
            throw std::runtime_error(ss.str());
        }

        auto cellname = insKp->getArchetypeName();
        auto cellKp = m_db.m_design.m_cellLib->lookupCell(cellname);

        if (!cellKp.isValid())
        {
            std::stringstream ss;
            ss << "  Cannot find cell of instance " << instance << "\n";
            throw std::runtime_error(ss.str());
        }

        auto cellsize = cellKp->m_size;
        Logging::logDebug("  CORNER %s %s %s\n", instance.c_str(), location.c_str(), cellname.c_str());

        if (location == "NE")
        {
            m_padring.m_upperLeftCorner.m_instanceName = instance;
            m_padring.m_upperLeftCorner.m_cellName = cellname;
            m_padring.m_upperLeftCorner.m_size = cellsize.m_x;
            m_padring.m_upperLeftCorner.m_itemType = LayoutItem::ItemType::CORNER;
            m_padring.m_upperLeftCorner.m_orientation = ChipDB::Orientation::R0;
        }
        else if (location == "NW")
        {
            m_padring.m_upperRightCorner.m_instanceName = instance;
            m_padring.m_upperRightCorner.m_cellName = cellname;
            m_padring.m_upperRightCorner.m_size = cellsize.m_x;
            m_padring.m_upperRightCorner.m_itemType = LayoutItem::ItemType::CORNER;
            m_padring.m_upperRightCorner.m_orientation = ChipDB::Orientation::R0;
        }
        else if (location == "SE")
        {
            m_padring.m_lowerLeftCorner.m_instanceName = instance;
            m_padring.m_lowerLeftCorner.m_cellName = cellname;
            m_padring.m_lowerLeftCorner.m_size = cellsize.m_x;
            m_padring.m_lowerLeftCorner.m_itemType = LayoutItem::ItemType::CORNER;
            m_padring.m_lowerLeftCorner.m_orientation = ChipDB::Orientation::R0;
        }
        else if (location == "SW")
        {
            m_padring.m_lowerRightCorner.m_instanceName = instance;
            m_padring.m_lowerRightCorner.m_cellName = cellname;
            m_padring.m_lowerRightCorner.m_size = cellsize.m_x;
            m_padring.m_lowerRightCorner.m_itemType = LayoutItem::ItemType::CORNER;
            m_padring.m_lowerRightCorner.m_orientation = ChipDB::Orientation::R0;
        }
        else
        {
            throw std::runtime_error("  CORNER unknown location type\n");
        }
    };

    void onPad(
        const std::string &instance,
        const std::string &location,
        bool flipped) override
    {
        auto modulePtr = m_db.m_design.getTopModule();
        if (!modulePtr)
        {
            throw std::runtime_error("Top module not set");
        }

        auto insKp = modulePtr->m_netlist->lookupInstance(instance);
        if (!insKp.isValid())
        {
            std::stringstream ss;
            ss << "  Cannot find instance " << instance << " for pad cell\n";
            throw std::runtime_error(ss.str());
        }

        auto cellname = insKp->getArchetypeName();
        auto cellKp = m_db.m_design.m_cellLib->lookupCell(cellname);

        if (!cellKp.isValid())
        {
            std::stringstream ss;
            ss << "  Cannot find cell of instance " << instance << "\n";
            throw std::runtime_error(ss.str());
        }

        auto cellsize = cellKp->m_size;
        Logging::logDebug("  PAD %s %s %s\n", instance.c_str(), location.c_str(), cellname.c_str());

        auto layoutItem = std::make_unique<LayoutItem>();
        layoutItem->m_instanceName = instance;
        layoutItem->m_cellName = cellname;
        layoutItem->m_itemType = LayoutItem::ItemType::CELL;
        layoutItem->m_size = cellsize.m_x;

        m_lastLocation = location;
        if (location == "N")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_top.insertLayoutItem(layoutItem.release());
        }
        else if (location == "W")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_left.insertLayoutItem(layoutItem.release());
        }
        else if (location == "E")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_right.insertLayoutItem(layoutItem.release());
        }
        else if (location == "S")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_bottom.insertLayoutItem(layoutItem.release());
        }
        else
        {
            throw std::runtime_error("  PAD unknown location type\n");
        }

    };

    void onFiller(const std::string &fillerName) override
    {
        auto module = m_db.m_design.getTopModule();
        if (!module)
        {
            throw std::runtime_error("Top module not set");
        }
    }

    /** callback for space in microns */
    void onSpace(double space) override
    {
        auto module = m_db.m_design.getTopModule();
        if (!module)
        {
            throw std::runtime_error("Top module not set");
        }

        auto layoutItem = std::make_unique<LayoutItem>();
        layoutItem->m_itemType = LayoutItem::ItemType::FIXEDSPACE;

        auto sz = static_cast<ChipDB::CoordType>(space*1000.0f);
        layoutItem->m_size = sz;
        layoutItem->m_size = sz;

        if (m_lastLocation == "N")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_top.insertLayoutItem(layoutItem.release());
        }
        else if (m_lastLocation == "W")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_left.insertLayoutItem(layoutItem.release());
        }
        else if (m_lastLocation == "E")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_right.insertLayoutItem(layoutItem.release());
        }
        else if (m_lastLocation == "S")
        {
            layoutItem->m_orientation = ChipDB::Orientation::R0;
            m_padring.m_bottom.insertLayoutItem(layoutItem.release());
        }
        else
        {
            throw std::runtime_error("  Space cannot be used before the first pad is specified\n");
        }

        Logging::logDebug("  SPACE %s %d nm\n", m_lastLocation.c_str(), sz);
    }

protected:
    std::string m_lastLocation;
    Database &m_db;
    Padring  &m_padring;
};


//bool place(Database &db, Padring &padring)

bool read(std::istream &is, Database &db, Padring &padring)
{
    PadringConfigReader reader(is, db, padring);

    try
    {
        if (!reader.parse())
        {
            return false;
        }
    }
    catch(const std::exception& e)
    {
        Logging::logError("%s\n", e.what());
        return false;
    }

    return true;
}

bool place(Database &db, Padring &padring)
{
    return padring.layout(db);
}

};
