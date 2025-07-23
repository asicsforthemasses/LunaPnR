// SPDX-FileCopyrightText: 2021-2025 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tasks.h"

namespace Tasks
{

class Place : public Task
{
public:
    Place() = default;
    virtual ~Place() = default;

protected:
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
