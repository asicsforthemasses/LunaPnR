##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
##

project(lunacore LANGUAGES CXX)

set(CMAKE_C_STANDARD 99)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UseCPPCHECK "UseCPPCHECK")
option(UseCLANGTIDY "UseCLANGTIDY")

########################################################################
# CPPCHECK static analysis support
########################################################################

if (UseCPPCHECK)
set(CMAKE_CXX_CPPCHECK "cppcheck")
    find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    message("Using CPPCHECK")
    list(
        APPEND CMAKE_CXX_CPPCHECK
            "--enable=warning"
            "--inconclusive"
            "--force"
            "--inline-suppr"
            "--suppressions-list=${CMAKE_CURRENT_SOURCE_DIR}/cppcheck/CppCheckSuppressions.txt"
    )
endif()

########################################################################
# CLANG TIDY
########################################################################
if (UseCLANGTIDY)
    find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy)
    message("Using CLANG TIDY")
    list(
        APPEND CMAKE_CXX_CLANG_TIDY
            "-checks=clang-diagnostic-*,clang-analyzer-*,modernize-*,-modernize-use-trailing-return-type"
    )
endif()

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
# Add TinySVGPP
########################################################################

CPMAddPackage("bb:trcwm/tinysvgpp#main")

########################################################################
# Add StrUtilsPP
########################################################################

CPMAddPackage("bb:trcwm/strutilspp#main")

########################################################################
# Add optional Python
########################################################################

option(USE_PYTHON "Enabled Python scripting" OFF)

if (USE_PYTHON)
    find_package (PythonLibs 3.8 REQUIRED COMPONENTS Interpreter Development)
    message(STATUS "Python scripting is ENABLED")
else(USE_PYTHON)
    message(STATUS "Python scripting is DISABLED")
endif(USE_PYTHON)

########################################################################
# Add Eigen3
########################################################################

find_package (Eigen3 3.3 REQUIRED NO_MODULE)

if (WIN32)
    set(PLATFORMSRC
        common/subprocess_win.cpp
    )
endif (WIN32)

if (UNIX)
    set(PLATFORMSRC
        common/subprocess_unix.cpp
    )
endif (UNIX)

########################################################################
# Add TomlPlusPlus
########################################################################

CPMAddPackage("gh:marzer/tomlplusplus#v3.3.0")

########################################################################
# Generate enums from the .toml specification
########################################################################
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/common/enums.h
    COMMAND $<TARGET_FILE:enumgen> ${CMAKE_CURRENT_SOURCE_DIR}/common/enums.toml ${CMAKE_CURRENT_SOURCE_DIR}/common/enums.h
    COMMENT "Generating enums ..."
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/common/enums.toml enumgen
)

if (USE_PYTHON)
    set (PYTHON_SRC
        python/pylunapnr.cpp
        python/converters.cpp
        python/consoleredirect.cpp
        python/types/pypin.cpp
        python/types/pynet.cpp
        python/types/pynets.cpp
        python/types/pycell.cpp
        python/types/pycelllib.cpp
        python/types/pypininfo.cpp
        python/types/pypininfolist.cpp
        python/types/pyinstance.cpp
        python/types/pyinstances.cpp
        python/types/pylayerinfo.cpp
        python/types/pytechlayers.cpp
        python/types/pysiteinfo.cpp
        python/types/pytechsites.cpp
    )
endif()

# main core
add_library(lunacore
    ../version/version.cpp
    common/logging.cpp
    common/fileutils.cpp
    common/matrix.cpp

    database/enums.h
    database/dbtypes.cpp
    database/geometry.cpp
    database/net.cpp
    database/instance.cpp
    database/netlist.cpp
    database/netlisttools.cpp
    database/cell.cpp
    database/pin.cpp
    database/module.cpp
    database/celllib.cpp
    database/techlib.cpp
    database/design.cpp
    database/floorplan.cpp
    database/region.cpp
    database/row.cpp

    cellplacer2/cellplacer2.cpp
    cellplacer2/fillerhandler.cpp
    cellplacer/qplacertypes.cpp
    #cellplacer/qplacer.cpp
    cellplacer/cellplacer.cpp
    #cellplacer/densitybitmap.cpp
    cellplacer/netlistsplitter.cpp
    cellplacer/qlaplacer_private.cpp
    cellplacer/qlaplacer.cpp
    cellplacer/rowlegalizer.cpp

    partitioner/fmpart.cpp
    import/liberty/libparser.cpp
    import/liberty/libreader.cpp
    import/liberty/libreaderimpl.cpp
    import/lef/lefparser.cpp
    import/lef/lefreaderimpl.cpp
    import/lef/lefreader.cpp
    import/def/defparser.cpp
    import/def/defreaderimpl.cpp
    import/def/defreader.cpp
    import/verilog/veriloglexer.cpp
    import/verilog/verilogparser.cpp
    import/verilog/verilogreader.cpp
    import/ppm/ppmreader.cpp
    export/svg/svgwriter.cpp
    export/dot/dotwriter.cpp
    export/def/defwriter.cpp
    export/ppm/ppmwriter.cpp
    export/verilog/verilogwriter.cpp
    export/txt/txtwriter.cpp
    export/spef/spefwriter.cpp
    ${PYTHON_SRC}
    globalroute/globalrouter.cpp
    globalroute/prim_private.cpp
    globalroute/prim.cpp
    globalroute/grid.cpp
    globalroute/wavefront.cpp
    cts/cts.cpp
    ${PLATFORMSRC}
    )

target_include_directories(lunacore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lunacore PRIVATE ${PYTHON_INCLUDE_DIRS})
target_include_directories(lunacore PUBLIC ../contrib)
target_include_directories(lunacore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lunacore Eigen3::Eigen ${PYTHON_LIBRARIES} tinysvgpp strutilspp tomlplusplus::tomlplusplus)

## utilties based on LunaPnR Core
add_subdirectory(utilities)
