/*
  LunaPnR Source Code
  
  SPDX-License-Identifier: GPL-3.0-only
  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
*/

#include "common/logging.h"
#include "defwriter.h"

bool LunaCore::DEF::write(std::ostream &os, const std::shared_ptr<ChipDB::Module> mod)
{
    auto writer = std::make_unique<LunaCore::DEF::Private::WriterImpl>(os);

    if (!mod->m_netlist)
    {
        Logging::doLog(Logging::LogType::WARNING, "DEF writer: module %s has no netlist\n", mod->name().c_str());
        return false;
    }

    writer->m_designName = mod->name();
    for(auto ins : mod->m_netlist->m_instances)
    {
        if (!writer->write(ins.ptr()))
        {
            Logging::doLog(Logging::LogType::ERROR,"DEF writer: failed to write file\n");
            return false;
        }
    }

    return true;
}

LunaCore::DEF::Private::WriterImpl::WriterImpl(std::ostream &os) : m_os(os)
{
}

LunaCore::DEF::Private::WriterImpl::~WriterImpl()
{
    m_os << "DESIGN " << m_designName << " ;\n";
    m_os << "UNITS DISTANCE MICRONS " << m_databaseUnits << " ; \n";
    m_os << "COMPONENTS " << m_cellCount << " ;\n";

    m_os << m_ss.str();

    m_os << "END COMPONENTS\n";
    m_os << "END DESIGN\n";
    Logging::doLog(Logging::LogType::VERBOSE, "DEF writer: exported %lu components\n", m_cellCount);
}


ChipDB::Coord64 LunaCore::DEF::Private::WriterImpl::toDEFCoordinates(const ChipDB::Coord64 &pos) const noexcept
{
    //return std::complex<double>(p.real(), m_height - p.imag());
    //FIXME: use database units defined in LEF file!

    int64_t dbunits = m_databaseUnits;
    if (m_databaseUnits == 0)
    {
        Logging::doLog(Logging::LogType::WARNING, "DEF database units not set! does your imported LEF file specify it?\n");
        Logging::doLog(Logging::LogType::WARNING, "  Assuming the value is 100.0\n");
        dbunits = 100;
    }

    return ChipDB::Coord64(pos.m_x * dbunits / 1000, pos.m_y * dbunits / 1000);
}

bool LunaCore::DEF::Private::WriterImpl::write(const std::shared_ptr<ChipDB::InstanceBase> instance)
{
    if (!instance)
    {
        return false;
    }

    if (instance->isPlaced())
    {
        m_ss << "  - " << instance->name() << " " << instance->getArchetypeName() << "\n";

        ChipDB::Coord64 defPos;
        switch(instance->m_orientation)
        {
        case ChipDB::Orientation::R0:
            defPos = toDEFCoordinates(instance->m_pos);
            m_ss << "    + PLACED ( " << defPos.m_x << " " << defPos.m_y << " ) ";
            m_ss << " N" << " ;\n";
            break;
        case ChipDB::Orientation::R180:     // for north edge        
            defPos = toDEFCoordinates(instance->m_pos);
            m_ss << "    + PLACED ( " << defPos.m_x << " " << defPos.m_y << " ) ";
            m_ss << " S" << " ;\n";
            break;
        case ChipDB::Orientation::R90:    // for west edge       
            //pos.m_y -= cell->m_size.m_x;
            defPos = toDEFCoordinates(instance->m_pos);
            m_ss << "    + PLACED ( " << defPos.m_x << " " << defPos.m_y << " ) ";
            m_ss << " W" << " ;\n";
            break;
        case ChipDB::Orientation::R270:    // for east edge
            //pos.m_x -= cell->m_size.m_y;
            //pos.m_y -= cell->m_size.m_x;
            defPos = toDEFCoordinates(instance->m_pos);
            m_ss << "    + PLACED ( " << defPos.m_x << " " << defPos.m_y << " ) ";
            m_ss << " E" << " ;\n";
            break;        
        case ChipDB::Orientation::MX:    // for east edge
            //pos.m_x -= cell->m_size.m_y;
            //pos.m_y -= cell->m_size.m_x;
            defPos = toDEFCoordinates(instance->m_pos);
            m_ss << "    + PLACED ( " << defPos.m_x << " " << defPos.m_y << " ) ";
            m_ss << " FS" << " ;\n";
            break;        
        case ChipDB::Orientation::MY:    // for east edge
            //pos.m_x -= cell->m_size.m_y;
            //pos.m_y -= cell->m_size.m_x;
            defPos = toDEFCoordinates(instance->m_pos);
            m_ss << "    + PLACED ( " << defPos.m_x << " " << defPos.m_y << " ) ";
            m_ss << " FN" << " ;\n";
            break;
        default:
            Logging::doLog(Logging::LogType::WARNING, "  defwriter: orientation %d not supported\n", static_cast<uint32_t>(instance->m_orientation));
            break;
        }

        m_cellCount++;
    }
    else
    {
        // what to do when cells are unplaced?
    }

    return true;
}

