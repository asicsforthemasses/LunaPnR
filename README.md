# Luna Place and Route

[![Build Status](https://www.travis-ci.com/asicsforthemasses/LunaPnR.svg?branch=stable)](https://www.travis-ci.com/asicsforthemasses/LunaPnR)

For more information, please visit the [homepage](https://www.asicsforthemasses.com "Luna PnR homepage").

This project is supported by [NLnet](https://nlnet.nl/).

## Build requirements
* GCC 9.3 or higher
* CMake 3.16 or higher
* Ninja-build
* Qt 5.12 or higher
* libboost-test-dev (for running tests)
* libboost-filesystem-dev (for running tests)
* cppcheck 2.4.1 or higher (optional)

## Building
* execute './bootstrap.sh', this will generate a ./build directory.
* enter the build directory and type 'ninja'.
* LunaPnR application will be available in ./gui/lunapnr