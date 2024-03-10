##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
##

cmake_minimum_required(VERSION 3.20)

OPTION(ENABLE_PACKAGING "ENABLE_PACKAGING Option" ON)
OPTION(USE_PYTHON "Enabled Python scripting" OFF)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

## Make Debian/Ubuntu happy: this way the executable is not detected as a shared lib :-/
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

########################################################################
# Check for CPM
########################################################################

set(CPM_DOWNLOAD_VERSION 0.38.2)

if(CPM_SOURCE_CACHE)
    set(CPM_DOWNLOAD_LOCATION "${CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
elseif(DEFINED ENV{CPM_SOURCE_CACHE})
    set(CPM_DOWNLOAD_LOCATION "$ENV{CPM_SOURCE_CACHE}/cpm/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
else()
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
    message(STATUS "Downloading CPM.cmake to ${CPM_DOWNLOAD_LOCATION}")
    file(DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake" ${CPM_DOWNLOAD_LOCATION}
    )
endif()

message("${CPMstatus}")

include(${CPM_DOWNLOAD_LOCATION})

########################################################################
# Add optional Python
########################################################################

if (USE_PYTHON)
    find_package (PythonLibs 3.8 REQUIRED COMPONENTS Interpreter Development)
    message(STATUS "Python scripting is ENABLED")
else(USE_PYTHON)
    message(STATUS "Python scripting is DISABLED")
endif(USE_PYTHON)

############################################################
## Setup LunaPnR version string
############################################################

project(luna_pnr VERSION 0.1.6 HOMEPAGE_URL https://www.asicsforthemasses.com)

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
        message(STATUS "!!!! Using flag -std:c++latest for MSVC !!!!")
    endif()
endif(MSVC)


############################################################
## Main project
############################################################

enable_testing()

include_directories(${PROJECT_SOURCE_DIR}/version)

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

