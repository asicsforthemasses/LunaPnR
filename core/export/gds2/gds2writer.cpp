#include "gds2writer.hpp"
#include "common/gds2defs.hpp"

namespace LunaCore::GDS2
{

bool write(std::ostream &os, const Database &database, const std::string &moduleName)
{
    using namespace LunaCore::GDS2::WriterImpl;

    auto moduleKp = database.m_design.m_moduleLib->lookupModule(moduleName);
    if (!moduleKp.isValid())
    {
        Logging::logError("module %s not found\n", moduleName.c_str());
        return false;
    }

    // HEADER record
    write16(os, 0x0006);    // Len = 6 bytes
    write16(os, 0x0002);    // HEADER id
    write16(os, 0x0003);    // version 3?

    // BGNLIB
    write16(os, 0x001C);    // Len
    write16(os, 0x0102);    // BGNLIB id
    write16(os, 0x0000);    // year (last modified)
    write16(os, 0x0000);    // month
    write16(os, 0x0000);    // day
    write16(os, 0x0000);    // hour
    write16(os, 0x0000);    // minute
    write16(os, 0x0000);    // second
    write16(os, 0x0000);    // year (last accessed)
    write16(os, 0x0000);    // month
    write16(os, 0x0000);    // day
    write16(os, 0x0000);    // hour
    write16(os, 0x0000);    // minute
    write16(os, 0x0000);    // second

    // LIBNAME
    write16(os, 0x0012);    // Len 18
    write16(os, 0x0206);    // LIBNAME id
    write16(os, 0x4141);
    write16(os, 0x4141);
    write16(os, 0x4141);
    write16(os, 0x4141);
    write16(os, 0x4141);
    write16(os, 0x4141);
    write16(os, 0x4141);

    // UNITS
    write16(os, 0x0014);        // two 8-byte real
    write16(os, 0x0305);        // UNITS id
    write32(os, 0x3E418937);
    write32(os, 0x4BC6A7EF);
    write32(os, 0x3944B82F);
    write32(os, 0xA09B5A54);

    // BGNSTR
    write16(os, 0x001C);    // Len
    write16(os, 0x0502);    // BGNSTR id
    write16(os, 0x0000);    // year (last modified)
    write16(os, 0x0000);    // month
    write16(os, 0x0000);    // day
    write16(os, 0x0000);    // hour
    write16(os, 0x0000);    // minute
    write16(os, 0x0000);    // second
    write16(os, 0x0000);    // year (last accessed)
    write16(os, 0x0000);    // month
    write16(os, 0x0000);    // day
    write16(os, 0x0000);    // hour
    write16(os, 0x0000);    // minute
    write16(os, 0x0000);    // second

    // STRNAME
    auto const designName = moduleKp->name();
    uint32_t bytes = designName.size() + (designName.size() % 2);

    write16(os, bytes+4);   // Len
    write16(os, 0x0606);    // STRNAME id
    write(os, designName);

    // write cells

    // write epilog

    // ENDSTR
    write16(os, 0x0004);    // Len = 4
    write16(os, 0x0700);    // ENDSTR id

    // ENDLIB
    write16(os, 0x0004);    // Len = 4
    write16(os, 0x0400);    // ENDLIB id

    return true;
};

};

namespace LunaCore::GDS2::WriterImpl
{

void write32(std::ostream &os, uint32_t value)
{
    uint8_t byte = static_cast<uint8_t>(value >> 24);
    os.write((const char*) &byte, 1);
    byte = static_cast<uint8_t>(value >> 16);
    os.write((const char*) &byte, 1);
    byte = static_cast<uint8_t>(value >> 8);
    os.write((const char*) &byte, 1);
    byte = static_cast<uint8_t>(value);
    os.write((const char*) &byte, 1);
}

void write16(std::ostream &os, uint16_t value)
{
    uint8_t byte = static_cast<uint8_t>(value >> 8);
    os.write((const char*) &byte, 1);
    byte = static_cast<uint8_t>(value);
    os.write((const char*) &byte, 1);
}

void write8(std::ostream &os, uint8_t value)
{
    os.write((const char*) &value, 1);
}

void writeFloat(std::ostream &os, float v)
{
    auto data = IEEE2GDSFloat(v);

    for(uint8_t byte : data.m_data)
    {
        write8(os, byte);
    }
}

std::size_t write(std::ostream &os, std::string_view view)
{
    std::size_t bytesWritten = 0;

    for(auto c : view)
    {
        write8(os, c);
        bytesWritten++;
    }

    if ((view.size() %2) == 1)
    {
        write8(os, 0);
        bytesWritten++;
    }

    return bytesWritten;
}

void write(std::ostream &os, const ChipDB::Instance &instance)
{
    double px = instance.m_pos.m_x;     // x-position in microns
    double py = instance.m_pos.m_y;     // y-position in microns

    uint32_t rot = 0;                   // rotation in degrees
    bool     flip = false;              // true if cell is to be flipped (GDS2 flipping style!)

#if 0
    // process regular cells that have N,S,E,W
    // locations
    if (item->m_location == "N")
    {
        // North orientation, rotation = 180 degrees
        if (item->m_flipped)
        {
            flip = true;
            rot = 0;
        }
        else
        {
            px += item->m_lefinfo->m_sx;
            rot = 180;
        }
    }
    else if (item->m_location == "S")
    {
        // South orientation, rotation = 0 degrees
        if (item->m_flipped)
        {
            flip = true;
            rot = 180;
            px += item->m_lefinfo->m_sx;
        }
        else
        {
            // nothing
        }
    }
    else if (item->m_location == "E")
    {
        if (item->m_flipped)
        {
            flip = true;
            rot = 270;
            py += item->m_lefinfo->m_sx;
        }
        else
        {
            rot = 90;
        }
    }
    else if (item->m_location == "W")
    {
        if (item->m_flipped)
        {
            flip = true;
            rot = 90;
        }
        else
        {
            py += item->m_lefinfo->m_sx;
            rot = 270;
        }
    }
    // process corner cells that have NE,NW,SE,SW locations
    else if (item->m_location == "NW")
    {
        rot = 270;
    }
    else if (item->m_location == "SE")
    {
        px += item->m_lefinfo->m_sy;
        rot = 90;
    }
    else if (item->m_location == "NE")
    {
        px += item->m_lefinfo->m_sx;
        rot = 180;
    }
    else if (item->m_location == "SW")
    {
        // nothing.
    }
#endif


    // SREF
    write16(os, 0x0004);    // Len
    write16(os, 0x0A00);    // SREF id

    // SNAME
    uint32_t bytes = instance.name().size() + (instance.name().size() % 2);
    write16(os, bytes+4);   // Len
    write16(os, 0x1206);    // SNAME
    write(os, instance.name());

    // check for FLIP
    if (flip)
    {
        write16(os, 0x0006);
        write16(os, 0x1A01);    // write STRANS
        write16(os, 0x8000);
    }
    else
    {
        write16(os, 0x0006);
        write16(os, 0x1A01);    // write STRANS
        write16(os, 0x0000);
    }

    // ANGLE
    if (rot != 0)
    {
        write16(os, 4+8);
        write16(os, 0x1C05);    // ANGLE id
        switch(rot)
        {
        case 90:
            write8(os, 2+64);       // exponent
            write32(os, 0x5A000000);// mantissa
            write8(os, 0);
            write8(os, 0);
            write8(os, 0);
            break;
        case 180:
            write8(os, 2+64);       // exponent
            write32(os, 0xB4000000);// mantissa
            write8(os, 0);
            write8(os, 0);
            write8(os, 0);
            break;
        case 270:
            write8(os, 3+64);       // exponent
            write32(os, 0x10E00000);// mantissa
            write8(os, 0);
            write8(os, 0);
            write8(os, 0);
            break;
        default:
            write8(os, 64);         // exponent
            write32(os, 0x00000000);// mantissa
            write8(os, 0);
            write8(os, 0);
            write8(os, 0);
        }
    }

    // XY
    write16(os, 4+8);
    write16(os, 0x1003);    // XY id
    write32(os, px);
    write32(os, py);

    // ENDEL
    write16(os, 4);         // Len
    write16(os, 0x1100);    // ENDEL id
}

};