#pragma once

#include "tasks.h"

namespace Tasks
{

class CheckTiming : public Task
{
public:
    CheckTiming() = default;
    virtual ~CheckTiming() = default;

protected:
    std::string createTCL(const GUI::Database &database, const std::string &topModuleName) const;
    void execute(GUI::Database &database, ProgressCallback callback) override;
};

};
