// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>
#include "common/logging.h"
#include "gds2reader.hpp"

namespace ChipDB::GDS2
{

bool Reader::read(std::istream &is, bool callbackForEachRecord)
{
    m_context.m_item = ItemType::UNDEFINED;

    bool done = false;
    while(!is.eof() && !done)
    {
        m_record.clear();

        // read record length
        uint16_t len = readUInt16(is);

        // record type and data type
        uint8_t recType  = readOctet(is);
        auto    dataType = static_cast<DataType>(readOctet(is));
        uint16_t recID   = static_cast<uint16_t>(recType) << 8;
        recID |= (static_cast<uint16_t>(dataType) & 0xFF);

        switch(recType)
        {
        case 0:
            Logging::logVerbose("HEADER (%u)\n", len);
            break;
        case 1:
            Logging::logVerbose("BGNLIB (%u)\n", len);
            break;
        case 2:
            Logging::logVerbose("LIBNAME (%u)\n", len);
            break;
        case 3:
            Logging::logVerbose("UNITS (%u)\n", len);
            break;
        case 4:
            Logging::logVerbose("ENDLIB (%u)\n", len);
            break;
        case 5:
            Logging::logVerbose("BGNSTR (%u)\n", len);
            break;
        case 6:
            Logging::logVerbose("STRNAME (%u)\n", len);
            break;
        case 7:
            Logging::logVerbose("ENDSTR (%u)\n", len);
            break;
        case 8:
            Logging::logVerbose("BOUNDARY (%u)\n", len);
            break;
        case 9:
            Logging::logVerbose("PATH (%u)\n", len);
            break;
        case 10:
            Logging::logVerbose("SREF (%u)\n", len);
            break;
        case 11:
            Logging::logVerbose("AREF (%u)\n", len);
            break;
        case 12:
            Logging::logVerbose("TEXT (%u)\n", len);
            break;
        case 13:
            Logging::logVerbose("LAYER (%u)\n", len);
            break;
        case 14:
            Logging::logVerbose("DATATYPE (%u)\n", len);
            break;
        case 15:
            Logging::logVerbose("WIDTH (%u)\n", len);
            break;
        case 16:
            Logging::logVerbose("XY (%u)\n", len);
            break;
        case 17:
            Logging::logVerbose("ENDEL (%u)\n", len);
            break;
        case 18:
            Logging::logVerbose("SNAME (%u)\n", len);
            break;
        case 19:
            Logging::logVerbose("COLROW (%u)\n", len);
            break;
        case 21:
            Logging::logVerbose("NODE (%u)\n", len);
            break;
        case 22:
            Logging::logVerbose("TEXTTYPE (%u)\n", len);
            break;
        case 23:
            Logging::logVerbose("PRESENTATION (%u)\n", len);
            break;
        case 25:
            Logging::logVerbose("STRING (%u)\n", len);
            break;
        case 26:
            Logging::logVerbose("STRANS (%u)\n", len);
            break;
        case 27:
            Logging::logVerbose("MAG (%u)\n", len);
            break;
        case 28:
            Logging::logVerbose("ANGLE (%u)\n", len);
            break;
        case 31:
            Logging::logVerbose("REFLIBS (%u)\n", len);
            break;
        case 32:
            Logging::logVerbose("FONTS (%u)\n", len);
            break;
        case 33:
            Logging::logVerbose("PATHTYPE (%u)\n", len);
            break;
        case 34:
            Logging::logVerbose("GENERATIONS (%u)\n", len);
            break;
        case 35:
            Logging::logVerbose("ATTRTABLE (%u)\n", len);
            break;
        case 36:
            Logging::logVerbose("STYPETABLE (%u)\n", len);
            break;
        case 37:
            Logging::logVerbose("STRTYPE (%u)\n", len);
            break;
        case 38:
            Logging::logVerbose("ELFLAGS (%u)\n", len);
            break;
        default:
            Logging::logVerbose("?? (%u)\n", len);
        }

        switch(dataType)
        {
        case DataType::NONE:
            Logging::logVerbose("  data: none\n");
            break;
        case DataType::BITARRAY:
            Logging::logVerbose("  data: bit array\n");
            break;
        case DataType::SIGNED16:
            Logging::logVerbose("  data: signed 16-bit int\n");
            break;
        case DataType::SIGNED32:
            Logging::logVerbose("  data: signed 32-bit int\n");
            break;
        case DataType::FLOAT32:
            Logging::logVerbose("  data: 32-bit float\n");
            break;
        case DataType::FLOAT64:
            Logging::logVerbose("  data: 64-bit float\n");
            break;
        case DataType::ASCIISTR:
            Logging::logVerbose("  data: ASCII string\n");
            break;
        default:
            Logging::logVerbose("  data: ??\n");
            break;
        }

        switch(dataType)
        {
        case DataType::ASCIISTR:
            m_string.clear();

            for(uint32_t i=4; i<len; i++)
            {
                uint8_t c = readOctet(is);
                if ((c >=32) && (c <= 127))
                {
                    m_string += c;
                }
            }
            break;
        case DataType::BITARRAY:
            {
                // bit array
                uint32_t items = (len-4)/2;
                m_data.resize(items);
                for(uint32_t i=0; i<items; i++)
                {
                    uint16_t bits = static_cast<uint16_t>(readOctet(is)) << 8;
                    bits |= static_cast<uint16_t>(readOctet(is));
                    m_data[i] = bits;
                }
            }
            break;
        case DataType::SIGNED16:
            {
                m_data.clear();
                uint32_t items = (len-4)/2;
                m_data.resize(items);
                for(uint32_t i=0; i<items; i++)
                {
                    int16_t v = readInt16(is);
                    m_data[i] = v;
                }
            }
            break;
        case DataType::SIGNED32:
            {
                m_data.clear();
                uint32_t items = (len-4)/4;
                m_data.resize(items);
                for(uint32_t i=0; i<items; i++)
                {
                    int32_t v = readInt32(is);
                    m_data[i] = v;
                }
            }
            break;
        case DataType::FLOAT64:
            {
                uint32_t items = (len-4)/8;
                m_fltdata.resize(items);
                for(uint32_t i=0; i<items; i++)
                {
                    m_fltdata[i] = readFloat8(is);
                }
            }
            break;
        case DataType::FLOAT32:
            Logging::logError("GDS: float32 is unsupported, recID = 0x%04X\n",
                recID);
            return false;
        default:
            // skip data of unknown type..
            for(uint32_t i=4; i<len; i++)
            {
                uint8_t c = readOctet(is);
            }
            break;
        }


        switch(recID)
        {
        case 0x0305:
            onUnits(m_fltdata[0], m_fltdata[1]);
            break;
        case 0x0400:
            onEndLibrary();
            done = true;
            break;
        case 0x0502:
            onStructure();
            break;
        case 0x0606:
            onStructureName(m_string);
            break;
        case 0x0700:
            onEndStructure();
            break;
        case 0x0D02:
            // layer
            m_context.m_layer = m_data[0];
            break;
        case 0x0E02:
            // data type
            m_context.m_dtype = m_data[0];
            break;
        case 0x0800:
            // boundary
            m_context.m_item = ItemType::BOUNDARY;
            onBoundaryBegin();
            break;
        case 0x0A00:
            // SREF
            onSREF();
            m_context.m_item = ItemType::SREF;
            break;
        case 0x0B00:
            // AREF
            onAREF();
            break;
        case 0x0C00:
            // text
            m_context.m_item = ItemType::TEXT;
            break;
        case 0x0900:
            //path
            m_context.m_item = ItemType::PATH;
            break;
        case 0x1003:
            // XY data
            {
                const int32_t N=m_data.size() / 2;
                m_coords.resize(N);
                for(int32_t i=0; i<N; i++)
                {
                    m_coords[i].m_x = m_data[i*2];
                    m_coords[i].m_y = m_data[i*2+1];
                }
            }
            break;
        case 0x1100:
            //end element -> handle all element callbacks
            {
                Coord32 pos;

                switch(m_context.m_item)
                {
                case ItemType::BOUNDARY:
                    onBoundaryEnd(m_context, m_coords);
                    break;
                case ItemType::TEXT:
                    onText(m_context, m_coords[0], m_string);
                    break;
                case ItemType::SREF:
                    onSREFXY(m_coords[0]);
                    break;
                default:
                    break;
                }
            }
            onEndElement(m_context);
            m_context.m_item = ItemType::UNDEFINED;
            break;
        case 0x1206:
            // structure reference name
            onSREFName(m_string);
            break;
        case 0x1A01:
            // STRANS
            {
                bool refl   = (m_data[0] & 0x8000) != 0;
                bool absmag = (m_data[0] & 0x0004) != 0;
                bool absrot = (m_data[0] & 0x0002) != 0;
                onSTRANS(refl, absmag, absrot);
            }
            break;
        case 0x1B05:
            // MAG
            onMAG(m_fltdata[0]);
            break;
        case 0x1C05:
            // ANGLE
            onANGLE(m_fltdata[0]);
            break;
        default:
            break;
        }

        if (callbackForEachRecord)
            onRecord(m_context, recID);

    }
    return true;
}

uint8_t Reader::readOctet(std::istream &is)
{
    uint8_t byte;

    is.read((char*)&byte, 1);
    m_record.push_back(byte);
    return byte;
}

uint32_t Reader::readUInt32(std::istream &is)
{
    uint32_t v = readOctet(is) << 24;
    v |= readOctet(is) << 16;
    v |= readOctet(is) << 8;
    v |= readOctet(is);
    return v;
}

uint16_t Reader::readUInt16(std::istream &is)
{
    uint16_t v = readOctet(is) << 8;
    v |= readOctet(is);
    return v;
}

int32_t Reader::readInt32(std::istream &is)
{
    int32_t v = readOctet(is) << 24;
    v |= readOctet(is) << 16;
    v |= readOctet(is) << 8;
    v |= readOctet(is);
    return v;
}

int16_t Reader::readInt16(std::istream &is)
{
    int16_t v = readOctet(is) << 8;
    v |= readOctet(is);
    return v;
}

double Reader::readFloat8(std::istream &is)
{
    double result = 0;
    uint8_t sexp = readOctet(is) ^ 0x40;
    uint64_t mantissa = static_cast<uint64_t>(readOctet(is)) << 48;
    mantissa |= static_cast<uint64_t>(readOctet(is)) << 40;
    mantissa |= static_cast<uint64_t>(readOctet(is)) << 32;
    mantissa |= static_cast<uint64_t>(readOctet(is)) << 24;
    mantissa |= static_cast<uint64_t>(readOctet(is)) << 16;
    mantissa |= static_cast<uint64_t>(readOctet(is)) << 8;
    mantissa |= static_cast<uint64_t>(readOctet(is));

    int8_t exponent = (sexp & 0x7F) | ((sexp << 1) & 0x80);
    //printf("%+" PRId64 " * 16^%d\n",mantissa, static_cast<int32_t>(exponent));

    result = static_cast<double>(mantissa)*std::pow(16.0, static_cast<double>(exponent))*std::pow(2.0, -56.0);
    return result;
}

}; //namespace
