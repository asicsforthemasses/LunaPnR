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
