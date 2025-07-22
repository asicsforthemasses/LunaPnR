##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022, 2024 Niels Moseley <asicsforthemasses@gmail.com>
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
# Add optional readline
########################################################################

find_package(Readline)
if (READLINE_FOUND)
    message("Using READLINE")
else()
    message("Skipping READLINE")
endif()

########################################################################
# Add TomlPlusPlus
########################################################################

CPMAddPackage("gh:marzer/tomlplusplus#v3.3.0")

########################################################################
# Generate enums from the .toml specification
########################################################################
add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/database/enums.h
    COMMAND $<TARGET_FILE:enumgen> ${CMAKE_CURRENT_SOURCE_DIR}/database/enums.toml ${CMAKE_CURRENT_SOURCE_DIR}/database/enums.h
    COMMENT "Generating enums ..."
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/database/enums.toml enumgen
)

# main core
add_library(lunacore
    ../version/version.cpp
    common/logging.cpp
    common/fileutils.cpp
    common/matrix.cpp
    common/gds2defs.cpp
    common/idiagnostics.cpp

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
    import/gds2/gds2reader.cpp
    export/svg/svgwriter.cpp
    export/gds2/gds2writer.cpp
    export/dot/dotwriter.cpp
    export/def/defwriter.cpp
    export/ppm/ppmwriter.cpp
    export/verilog/verilogwriter.cpp
    export/txt/txtwriter.cpp
    export/spef/spefwriter.cpp

    passes/pass.cpp
    passes/passes.cpp
    passes/gdsmerge.cpp

    padring/configreader.cpp
    padring/padring.cpp
    padring/padringplacer.cpp

    globalroute/globalrouter.cpp
    globalroute/prim_private.cpp
    globalroute/prim.cpp
    globalroute/grid.cpp
    globalroute/wavefront.cpp
    cts/cts.cpp
    ${PLATFORMSRC}
    )

target_include_directories(lunacore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lunacore PUBLIC ../contrib)
target_include_directories(lunacore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(lunacore tinysvgpp strutilspp tomlplusplus::tomlplusplus)

## main: lunapnrcon console version
add_executable(lunapnrcon main.cpp)
target_include_directories(lunapnrcon PRIVATE ${Readline_INCLUDE_DIR})
target_link_libraries(lunapnrcon lunacore ${Readline_LIBRARY})
target_compile_definitions(lunapnrcon PRIVATE -DLUNAVERSIONSTRING=${SHORTLUNAVERSIONSTRING})

if (READLINE_FOUND)
    target_compile_definitions(lunapnrcon PRIVATE USE_READLINE)
endif()

## utilties based on LunaPnR Core
add_subdirectory(utilities)
