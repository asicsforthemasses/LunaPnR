#include <cstdlib>
#include "lunacore.h"

int main(int argc, char *argv[])
{
    Scripting::Python p;

    p.executeScript("import Luna\nprint('Hello from Python!')");

    return EXIT_SUCCESS;
}

