// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include "version.h"
#include "common/logging.h"
#include "defwriter.h"

bool LunaCore::DEF::write(std::ostream &os, const std::shared_ptr<ChipDB::Module> mod,
    const WriterOptions &options)
{
    auto writer = std::make_unique<LunaCore::DEF::Private::WriterImpl>(os);

    if (!mod->m_netlist)
    {
        Logging::logWarning("DEF writer: module %s has no netlist\n", mod->name().c_str());
        return false;
    }

    std::size_t skippedFillers = 0;
    std::size_t skippedDecap   = 0;
    writer->m_designName = mod->name();
    for(auto ins : mod->m_netlist->m_instances)
    {
        if ((!options.exportFillers) && (ins->isCoreFiller()))
        {
            skippedFillers++;
            continue;
        }

        if ((!options.exportDecap) && (ins->isCoreDecap()))
        {
            skippedDecap++;
            continue;
        }

        if (!writer->write(ins.ptr()))
        {
            Logging::logError("DEF writer: failed to write file\n");
            return false;
        }
    }

    Logging::logVerbose("DEF writer: skipped %ul filler cells and %ul decap cells",
        skippedFillers, skippedDecap);

    return true;
}

bool LunaCore::DEF::write(std::ostream &os, const std::shared_ptr<ChipDB::Module> mod)
{
    WriterOptions options;
    return write(os, mod, options);
}

LunaCore::DEF::Private::WriterImpl::WriterImpl(std::ostream &os) : m_os(os)
{
}

LunaCore::DEF::Private::WriterImpl::~WriterImpl()
{
    m_os << "# Generated with " << LUNAVERSIONSTRING << "\n\n";
    m_os << "VERSION 5.4 ;\n";
    m_os << "BUSBITCHARS \"[]\" ;\n";
    m_os << "DIVIDERCHAR \"/\" ;\n";
    m_os << "DESIGN " << m_designName << " ;\n";
    m_os << "UNITS DISTANCE MICRONS " << m_databaseUnits << " ; \n";
    m_os << "COMPONENTS " << m_cellCount << " ;\n";

    m_os << m_ss.str();

    m_os << "END COMPONENTS\n";
    m_os << "END DESIGN\n";
    Logging::logVerbose("DEF writer: exported %lu components\n", m_cellCount);
}


ChipDB::Coord64 LunaCore::DEF::Private::WriterImpl::toDEFCoordinates(const ChipDB::Coord64 &pos) const noexcept
{
    //return std::complex<double>(p.real(), m_height - p.imag());
    //FIXME: use database units defined in LEF file!

    int64_t dbunits = m_databaseUnits;
    if (m_databaseUnits == 0)
    {
        Logging::logWarning("DEF database units not set! does your imported LEF file specify it?\n");
        Logging::logWarning("  Assuming the value is 100.0\n");
        dbunits = 100;
    }

    return {pos.m_x * dbunits / 1000, pos.m_y * dbunits / 1000};
}

bool LunaCore::DEF::Private::WriterImpl::write(const std::shared_ptr<ChipDB::Instance> instance)
{
    if (!instance)
    {
        return false;
    }

    if (instance->isPlaced())
    {
        m_ss << "  - " << instance->name() << " " << instance->getArchetypeName() << "\n";

        ChipDB::Coord64 defPos;
        switch(instance->m_orientation.value())
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
            Logging::logWarning("  defwriter: orientation %s not supported\n", instance->m_orientation.toString().c_str());
            break;
        }

        m_cellCount++;
    }
    else
    {
        m_ss << "  - " << instance->name() << " " << instance->getArchetypeName() << "\n";
        m_ss << "    + UNPLACED ;\n";
        m_cellCount++;
    }

    return true;
}

