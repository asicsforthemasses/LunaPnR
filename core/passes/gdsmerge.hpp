// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once
#include <fstream>
#include <filesystem>
#include "common/logging.h"
#include "pass.hpp"
#include "import/import.h"

namespace LunaCore::Passes
{

class GDS2Merger : public ChipDB::GDS2::Reader
{
public:
    bool read(const std::string &inFilena);
    bool write(const std::string &outFilename);

    void onRecord(const context_t &context, uint16_t recID) override;

protected:

    struct record_t
    {
        uint16_t                m_id;       ///< GDS2 record ID
        std::vector<uint8_t>    m_data;     ///< raw GDS2 data bytes
    };

    std::vector<record_t> m_records;        ///< raw GDS2 records
};

class GDSMergePass : public Pass
{
public:
    GDSMergePass() : Pass("gdsmerge")
    {
        registerNamedParameter("output", "", 1, true);
    }

    virtual ~GDSMergePass() = default;

    /** execute a pass given a list of input arguments.
        returns true if succesful, else false.
    */
    [[nodiscard]] bool execute(Database &database) override
    {
        if (m_params.size() < 2)
        {
            Logging::logError("gdsmerge requires at least two .gds input files\n");
            return false;
        }

        if (!m_namedParams.contains("output"))
        {
            Logging::logError("gdsmerge requires an output file, use -output\n");
            return false;
        }

        auto merger = std::make_unique<GDS2Merger>();

        for(auto const &infileName : m_params)
        {
            if (!merger->read(infileName)) return false;
        }

        if (!merger->write(m_namedParams.at("output").front()))
        {
            return false;
        }

        return true;
    }

    /**
        returns help text for a pass.
    */
    std::string help() const noexcept override
    {
        std::stringstream ss;
        ss << "gdsmerge - merge two or more GDS2 files\n";
        ss << "  gdsmerge -output <output file> <input files ..>\n\n";
        ss << "\n";
        return ss.str();
    }

    /**
        returns a one-line short help text for a pass.
    */
    virtual std::string shortHelp() const noexcept
    {
        return "merge two or more GDS2 files";
    }

    /**
        Initialize a pass. this is called by registerPass()
    */
    bool init() override
    {
        return true;
    }
};


};
