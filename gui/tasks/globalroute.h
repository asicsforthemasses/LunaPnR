// SPDX-FileCopyrightText: 2021-2022 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tasks.h"

namespace Tasks
{

class GlobalRoute : public Task
{
public:
    GlobalRoute() = default;
    virtual ~GlobalRoute() = default;

protected:
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
