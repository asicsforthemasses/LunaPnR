// SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "tasks.h"

namespace Tasks
{

class CreateFloorplan : public Task
{
public:
    CreateFloorplan() = default;
    virtual ~CreateFloorplan() = default;

protected:
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
