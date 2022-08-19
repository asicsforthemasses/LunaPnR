#include <chrono>
#include "place.h"

void Tasks::Place::execute(GUI::Database &database, ProgressCallback callback)
{
    m_status.store(Status::RUNNING);

    auto topModule = database.design().getTopModule();

    if (!topModule)
    {
        error("Top module not set\n");
        return;
    }    

    std::this_thread::sleep_for (std::chrono::seconds(6));

    done();
}
