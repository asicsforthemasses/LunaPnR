// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tasks.h"

namespace Tasks
{

class CheckTiming : public Task
{
public:

    enum class Mode
    {
        BARE,
        CTS,
        WITHSPEF
    };

    CheckTiming(Mode mode) : m_mode(mode) {}

    virtual ~CheckTiming() = default;

protected:
    std::string createTCL(const GUI::Database &database, 
        const std::string &topModuleName,
        const std::string &spefFilename,
        const std::string &verilogFilename) const;
        
    void execute(GUI::Database &database, ProgressCallback callback) override;

    Mode m_mode{Mode::BARE};
};

};
