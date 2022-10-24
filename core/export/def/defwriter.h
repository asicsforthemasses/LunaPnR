// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <iostream>
#include <sstream>
#include <memory>
#include "celllib/pin.h"
#include "netlist/instance.h"
#include "netlist/netlist.h"

namespace LunaCore::DEF
{
    struct WriterOptions
    {
        bool exportFillers{true};   ///< if true, it exports filler cells
        bool exportDecap{true};     ///< if true, it exports decap cells
    };

    /** write a module as a DEF file - exports depend on the options. */
    bool write(std::ostream &os, const std::shared_ptr<ChipDB::Module> module,
        const WriterOptions &options);

    /** write a module as a DEF file - exports everything. */
    bool write(std::ostream &os, const std::shared_ptr<ChipDB::Module> module);
};

namespace LunaCore::DEF::Private
{
    class WriterImpl
    {
    public:
        WriterImpl(std::ostream &os);
        virtual ~WriterImpl();

        ChipDB::Coord64 toDEFCoordinates(const ChipDB::Coord64 &pos) const noexcept;

        bool write(const std::shared_ptr<ChipDB::Instance> instance);

        size_t  m_cellCount     = 0;
        int64_t m_databaseUnits = 100;  /** NOTE: default, actual should come from from LEF file */

        std::stringstream   m_ss;
        std::ostream        &m_os;
        std::string         m_designName;
    };

    

};
