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
