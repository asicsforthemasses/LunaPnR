#pragma once

#include "tasks.h"

namespace Tasks
{

class CheckTiming : public Task
{
public:
    CheckTiming() : Task("CheckTiming") {}

protected:
    std::string createTCL(const GUI::Database &database, const std::string &topModuleName) const;
    void execute(GUI::Database &database) override;
};

};
