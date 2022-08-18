#pragma once

#include "tasks.h"

namespace Tasks
{

class PreflightChecks : public Task
{
public:
    PreflightChecks() : Task("PreflightChecks") {}

protected:
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
