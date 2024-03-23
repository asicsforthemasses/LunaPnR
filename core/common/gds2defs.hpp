#pragma once
#include <cstdint>

namespace LunaCore::GDS2
{

    enum class Item_t
    {
        I_UNDEFINED,
        I_BOUNDARY,
        I_PATH,
        I_TEXT,
        I_NODE,
        I_SREF,
        I_AREF,
        I_BOX,
        I_PROPERTY
    };

    enum class RecID : uint16_t
    {
        HEADER   = 0x0002,
        BGNLIB   = 0x0102,
        LIBNAME  = 0x0206,
        UNITS    = 0x0305,
        ENDLIB   = 0x0400,
        BGNSTR   = 0x0502,
        STRNAME  = 0x0606,
        ENDSTR   = 0x0700,
        BOUNDARY = 0x0800,
        PATH     = 0x0900,
        SREF     = 0x0A00,
        AREF     = 0x0B00,
        TEXT     = 0x0C00,
        LAYER    = 0x0D02,
        DATATYPE = 0x0E02,
        WIDTH    = 0x0F03,
        XY       = 0x1003,
        ENDEL    = 0x1100,
        SNAME    = 0x1206,
        COLROW   = 0x1302,
        NODE     = 0x1500,
        TEXTTYPE = 0x1602,
        PRESENTATION = 0x1701,
        STRING   = 0x1906,
        STRANS   = 0x1A01,
        MAG      = 0x1B05,
        ANGLE    = 0x1C05,
        REFLIBS  = 0x1F06,
        FONTS    = 0x2006,
        PATHTYPE = 0x2102,
        GENERATIONS = 0x2202,
        ATTRTABLE = 0x2306,
        ELFLAGS  = 0x2601,
        NODETYPE = 0x2A02,
        PROPATTR = 0x2B02,
        PROPVALUE = 0x2C06,
        BOX      = 0x2D00,
        BOXTYPE  = 0x2E02,
        PLEX     = 0x2F03,
        TAPENUM  = 0x3202,
        TAPECODE = 0x3302,
        FORMAT   = 0x3602,
        MASK     = 0x3706,
        ENDMASKS = 0x3800
    };

    struct GDS2Float
    {
        uint8_t m_data[8];
    };

    GDS2Float IEEE2GDSFloat(double value);
    double GDS2Float2IEEE(const GDS2Float &value);

};