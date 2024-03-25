// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <istream>
#include <cstdint>
#include "common/gds2defs.hpp"
#include "database/database.h"

/** Namespace for the LEF importers and exporters */
namespace ChipDB::GDS2
{

class Reader
{
public:
    virtual bool read(std::istream &is, bool callbackForEachRecord = false);

    struct Coord32
    {
        int32_t m_x{0};
        int32_t m_y{0};
    };

    enum class ItemType
    {
        UNDEFINED,
        BOUNDARY,
        PATH,
        TEXT,
        NODE,
        SREF,
        AREF,
        BOX,
        PROPERTY
    };

    struct context_t
    {
        ItemType    m_item{ItemType::UNDEFINED};
        uint32_t    m_layer{0};     ///< layer number
        uint32_t    m_dtype{0};     ///< GDS2 data type number
    };

    /** callback for boundaries (at end of record) */
    virtual void onBoundaryEnd(const context_t &context,
        const std::vector<Coord32> &points) {};

    /** callback for boundaries (at beginning of record) */
    virtual void onBoundaryBegin() {};

    /** callback for structures */
    virtual void onStructure() {};

    /** callbnack for structure name */
    virtual void onStructureName(const std::string &name) {};

    /** callback for end structure */
    virtual void onEndStructure() {};

    /** callback for units */
    virtual void onUnits(double userunits, double dbunits) {};

    /** callback for text */
    virtual void onText(const context_t &context,
        const Coord32 &pos,
        const std::string &txt) {};

    /** end of library */
    virtual void onEndLibrary() {};

    /** end of element */
    virtual void onEndElement(const context_t &context) {};

    /** callback for a structure reference */
    virtual void onSREF() {};

    /** callback for structure reference name */
    virtual void onSREFName(const std::string &name) {};

    /** callback for SREF position */
    virtual void onSREFXY(const Coord32 &pos) {};

    /** callback for structure transformation info */
    virtual void onSTRANS(bool reflect, bool absAngle, bool absMag) {};

    /** callback for magnification factor */
    virtual void onMAG(double mag) {};

    /** callback for angle of rotation.
     *  counter-clockwise, in degrees.
    */
    virtual void onANGLE(double angle) {};

    /** callback for XY data */
    virtual void onXY(const std::vector<Coord32> &points) {};

    /** callback for a structure reference */
    virtual void onAREF() {};

    /** callback for each record.
     *  if needed, use getRawRecord() to access the data externally.
     */
    virtual void onRecord(const context_t &context, uint16_t recID) {};

    /** access the raw record data */
    const std::vector<uint8_t>& getRawRecord() const
    {
        return m_record;
    }

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

    enum class DataType : int8_t
    {
        NONE = 9,
        BITARRAY = 1,
        SIGNED16 = 2,
        SIGNED32 = 3,
        FLOAT32  = 4,
        FLOAT64  = 5,
        ASCIISTR = 6
    };

protected:
    uint8_t readOctet(std::istream &is);
    double  readFloat8(std::istream &is);
    int32_t readInt32(std::istream &is);
    int16_t readInt16(std::istream &is);
    uint32_t readUInt32(std::istream &is);
    uint16_t readUInt16(std::istream &is);

    std::vector<uint8_t> m_record;  ///< raw record data
    std::vector<int32_t> m_data;    ///< integer record arguments
    std::vector<double>  m_fltdata; ///< floating point record arguments
    std::vector<Coord32> m_coords;  ///< coordinate buffer for XY and positional info
    std::string m_string;           ///< string record argument

    context_t   m_context;
};

};
