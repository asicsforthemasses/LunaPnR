// SPDX-FileCopyrightText: 2021-2024 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tasks.h"

namespace Tasks
{

class CTS : public Task
{
public:
    CTS() = default;
    virtual ~CTS() = default;

protected:
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
