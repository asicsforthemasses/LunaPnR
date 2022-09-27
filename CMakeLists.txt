##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
##  

cmake_minimum_required(VERSION 3.20)

OPTION(ENABLE_PACKAGING "ENABLE_PACKAGING Option" OFF)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Make Debian/Ubuntu happy: this way the executable is not detected as a shared lib :-/
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

############################################################
## Setup LunaPnR version string
############################################################

project(luna_pnr VERSION 0.1.4 HOMEPAGE_URL https://www.asicsforthemasses.com)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("Building debug binaries")
    set(LUNAVERSION "\"LunaPnR version ${CMAKE_PROJECT_VERSION} DEBUG\"")
ELSE()
    set(LUNAVERSION "\"LunaPnR version ${CMAKE_PROJECT_VERSION}\"")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

message(${LUNAVERSION})

add_definitions(-DLUNAVERSIONSTRING=${LUNAVERSION})

############################################################
## Setup Compiler version string
############################################################

set (COMPILERVERSION "\"${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}\"")
add_definitions(-DCOMPILERVERSIONSTRING=${COMPILERVERSION})

############################################################
## UNIX specific options
############################################################
if(UNIX)
    set(LUNA_INSTALL_PREFIX "/opt/lunapnr-${CMAKE_PROJECT_VERSION}")
    set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

    if(ENABLE_PACKAGING)
        include(Packing)
    endif(ENABLE_PACKAGING)
endif(UNIX)

############################################################
## Microsoft Visual studio (window) specific options
############################################################

if (MSVC)
    add_definitions(-DNO_SSIZE_T -DEIGEN_HAS_STD_INVOKE_RESULT)
    add_compile_options(/EHsc)

    # required for VS2019
    if (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 19.12.25835)
        set(CMAKE_CXX17_STANDARD_COMPILE_OPTION "-std:c++latest")
        set(CMAKE_CXX17_EXTENSION_COMPILE_OPTION "-std:c++latest")
    endif()
endif(MSVC)


############################################################
## Main project
############################################################

enable_testing()

add_subdirectory(contrib)
add_subdirectory(tools)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(test)

############################################################
## Unix install specifics
############################################################

if(UNIX)
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/gui/lunapnr
        DESTINATION ${LUNA_INSTALL_PREFIX}/bin)
endif(UNIX)

