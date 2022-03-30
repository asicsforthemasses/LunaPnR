##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
##  

cmake_minimum_required(VERSION 3.20)

## Make Debian/Ubuntu happy: this way the executable is not detected as a shared lib :-/
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

project(core_python)

add_executable(core_python main.cpp)
target_include_directories(core_python PRIVATE ../../core/include)
target_link_libraries(core_python lunacore)