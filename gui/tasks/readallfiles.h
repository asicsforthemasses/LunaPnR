#pragma once

#include "tasks.h"

namespace Tasks
{

class ReadAllFiles : public Task
{
public:
    ReadAllFiles() = default;
    virtual ~ReadAllFiles() = default;

protected:
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
