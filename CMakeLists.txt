##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
##  

cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Make Debian/Ubuntu happy: this way the executable is not detected as a shared lib :-/
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

############################################################
## Setup LunaPnR version string
##
## inspired by https://jonathanhamberg.com/post/cmake-embedding-git-hash/
##
############################################################

project(luna_pnr VERSION 0.1.5 HOMEPAGE_URL https://www.asicsforthemasses.com)

IF(CMAKE_BUILD_TYPE MATCHES Debug)
    message("!!!! Building debug binaries !!!!")
    set(LUNAVERSIONSTRING "LunaPnR version ${CMAKE_PROJECT_VERSION} DEBUG")
ELSE()
    set(LUNAVERSIONSTRING "LunaPnR version ${CMAKE_PROJECT_VERSION}")
ENDIF(CMAKE_BUILD_TYPE MATCHES Debug)

message(STATUS LunaPnR Version: ${LUNAVERSIONSTRING})

configure_file(${PROJECT_SOURCE_DIR}/version/version.cpp.in ${PROJECT_SOURCE_DIR}/version/version.cpp @ONLY)

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
    include(Packing)
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
        message(STATUS "!!!! Using flag -std:c++latest for MSVC !!!!")
    endif()
endif(MSVC)


############################################################
## Main project
############################################################

enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/version)

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

