# LunaPnR CMAKE file for LUA

project(lua C)
cmake_minimum_required(VERSION 3.20)

## Make Debian/Ubuntu happy: this way the executable is not detected as a shared lib :-/
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

set(SRC
    src/lapi.c
    src/lauxlib.c
    src/lbaselib.c
    src/lcode.c
    src/lcorolib.c
    src/lctype.c
    src/ldblib.c
    src/ldebug.c
    src/ldo.c
    src/ldump.c
    src/lfunc.c
    src/lgc.c
    src/linit.c
    src/liolib.c
    src/llex.c
    src/lmathlib.c
    src/lmem.c
    src/loadlib.c
    src/lobject.c
    src/lopcodes.c
    src/loslib.c
    src/lparser.c
    src/lstate.c
    src/lstring.c
    src/lstrlib.c
    src/ltable.c
    src/ltablib.c
    src/ltm.c
    src/lundump.c
    #src/lua.c
    src/lutf8lib.c
    src/lvm.c
    src/lzio.c
)

add_library(liblua ${SRC})
target_include_directories(liblua PRIVATE src)
