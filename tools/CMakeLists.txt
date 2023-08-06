## SPDX-FileCopyrightText: 2021-2023 Niels Moseley <asicsforthemasses@gmail.com>
##
## SPDX-License-Identifier: GPL-3.0-only

project(doctools)

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
# Add StrUtilsPP
########################################################################

CPMAddPackage("bb:trcwm/strutilspp#main")

########################################################################
# Tools
########################################################################

add_executable(doctool ${PROJECT_SOURCE_DIR}/doctool.cpp)
add_executable(fixme ${PROJECT_SOURCE_DIR}/fixme.cpp)
add_executable(spdx ${PROJECT_SOURCE_DIR}/spdx.cpp)
add_executable(enumgen ${PROJECT_SOURCE_DIR}/enumgen.cpp)
target_link_libraries(enumgen tomlpp cxxopts strutilspp)
