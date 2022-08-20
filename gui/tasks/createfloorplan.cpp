#include "createfloorplan.h"
#include "../python/pyluna_extra.h"

void Tasks::CreateFloorplan::execute(GUI::Database &database, ProgressCallback callback)
{
    auto python = std::make_unique<GUI::Python>(&database, nullptr);
    python->init();

    // install python console redirection
    python->setConsoleRedirect(
        [this](const char *txt, ssize_t strLen)
        {
            if ((txt == nullptr)  || (strLen <= 0))
            {
                return;
            }            

            std::string strtxt(txt, strLen);
            info(strtxt);
        },
        [this](const char *txt, ssize_t strLen)
        {
            if ((txt == nullptr)  || (strLen <= 0))
            {
                return;
            }            

            std::string strtxt(txt, strLen);
            error(strtxt);
        }        
    );

    python->executeScript(R"(import sys; print("Python version"); print (sys.version); )");
    python->executeScript(R"(from Luna import *; from LunaExtra import *;)");

    auto script = R"(
## create floorplan
createRegion("core", 10000, 10000, 80000, 80000)
createRows("core", 0, 10000, 8)

## place the pins
xpos = 0
xinc = 10000
x = 10000
for idx in range(0,4): 
    pinName = "a_in[" + str(idx) + "]"
    placeInstance(pinName, "multiplier", x, 90000)
    x = x + xinc
    pinName = "b_in[" + str(idx) + "]"
    placeInstance(pinName, "multiplier", x, 90000)
    x = x + xinc    

xinc = 10000
x = 10000
for idx in range(0,8):
    pinName = "data_out[" + str(idx) + "]"
    placeInstance(pinName, "multiplier", x, 10000)
    x = x + xinc
)";    

    python->executeScript(script);

    info("createfloorplan task finished\n");

    done();
}
