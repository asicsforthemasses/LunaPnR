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
