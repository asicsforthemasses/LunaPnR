// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "defparser.h"
#include "common/dbtypes.h"
#include "design/design.h"

namespace ChipDB::DEF
{

class ReaderImpl : public Parser
{
public:
    ReaderImpl(Design &design);

    void onDesign(const std::string &designName) override;
    void onEndDesign(const std::string &designName) override;
    void onComponent(const std::string &insName, const std::string &archetype) override;
    void onComponentPlacement(const ChipDB::Coord64 &pos, 
        const ChipDB::PlacementInfo placement,
        const ChipDB::Orientation orient) override;

    void onEndParse() override;

protected:
    Design     &m_design;
};

}; //namespace
