#pragma once

#include "tasks.h"

namespace Tasks
{

class ReadAllFiles : public Task
{
public:
    
protected:
    void execute(GUI::Database &database) override;
};

};
