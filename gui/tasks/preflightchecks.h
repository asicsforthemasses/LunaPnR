// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tasks.h"

namespace Tasks
{

class PreflightChecks : public Task
{
public:
    PreflightChecks() = default;
    virtual ~PreflightChecks() = default;

protected:
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
