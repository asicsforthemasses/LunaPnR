#include "common/fileutils.h"
#include "gdsmerge.hpp"

namespace LunaCore::Passes
{

bool GDS2Merger::read(const std::string &inFilename)
{
    if (!LunaCore::fileExists(inFilename))
    {
        Logging::logError("'%s' does not exists\n", inFilename.c_str());
        return false;
    }

    auto oldRectCount = m_records.size();

    std::ifstream infile(inFilename);
    if (!ChipDB::GDS2::Reader::read(infile, true))
    {
        Logging::logError("Failed to parse '%s'\n", inFilename.c_str());
        return false;
    }

    Logging::logInfo("Read %ld records from %s\n", m_records.size() - oldRectCount,
        inFilename.c_str());
    return true;
}

bool GDS2Merger::write(const std::string &outFilename)
{
    using RecID = ChipDB::GDS2::Reader::RecID;

    std::ofstream outfile(outFilename);
    if (!outfile.is_open())
    {
        Logging::logError("Cannot open file %s for writing\n", outFilename.c_str());
        return false;
    }

    bool first = true;
    bool inStructure = false;
    for(auto const& rec : m_records)
    {
        // only write the headers from the first file
        // and skip ENDLIB
        auto recID = static_cast<RecID>(rec.m_id);
        if (first)
        {
            if (recID != RecID::ENDLIB)
            {
                outfile.write((char*)&rec.m_data[0], rec.m_data.size());
            }
            else
            {
                first = false;
            }
        }
        else
        {
            switch(recID)
            {
            case RecID::BGNSTR:
                inStructure = true;
                outfile.write((char*)&rec.m_data[0], rec.m_data.size());
                break;
            case RecID::ENDSTR:
                inStructure = false;
                outfile.write((char*)&rec.m_data[0], rec.m_data.size());
                break;
            default:
                if (inStructure)
                {
                    outfile.write((char*)&rec.m_data[0], rec.m_data.size());
                }
                break;
            }
        }
    }

    // write ENDLIB chunk
    const std::array<uint8_t, 4> epilog = {0x00, 0x04, 0x04, 0x00};

    outfile.write((char*)epilog.data(), epilog.size());
    Logging::logError("Written %lu records\n", m_records.size());
    return true;
}

void GDS2Merger::onRecord(const context_t &context, uint16_t recID)
{
    // this callback gets called for _every_ record!
    // the record is stored in m_

    auto &rec = m_records.emplace_back();
    rec.m_data = m_record;
    rec.m_id   = recID;
}

};
