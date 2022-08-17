#pragma once

#include "tasks.h"

namespace Tasks
{

class ReadAllFiles : public Task
{
public:
    ReadAllFiles() : Task("ReadAllFiles") {}

protected:
    void execute(GUI::Database &database) override;
};

};
