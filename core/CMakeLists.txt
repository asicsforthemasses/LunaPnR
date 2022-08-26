##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022 Niels Moseley <asicsforthemasses@gmail.com>
##  

project(lunacore)

set(CMAKE_CXX_STANDARD C99)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(UseCPPCHECK "UseCPPCHECK")
option(UseCLANGTIDY "UseCLANGTIDY")

# CPPCHECK static analysis support
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

# CLANG TIDY
if (UseCLANGTIDY)
    find_program(CMAKE_CXX_CLANG_TIDY NAMES clang-tidy)
    message("Using CLANG TIDY")
    list(
        APPEND CMAKE_CXX_CLANG_TIDY
            "-checks=clang-diagnostic-*,clang-analyzer-*,modernize-*,-modernize-use-trailing-return-type"
    )
endif()

find_package (Eigen3 3.3 REQUIRED NO_MODULE)
find_package (PythonLibs 3.8 REQUIRED COMPONENTS Interpreter Development)

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

add_library(lunacore 
    common/logging.cpp
    common/dbtypes.cpp
    common/fileutils.cpp
    netlist/net.cpp
    netlist/instance.cpp
    netlist/netlist.cpp
    netlist/netlisttools.cpp
    cellplacer2/cellplacer2.cpp
    cellplacer/qplacertypes.cpp
    cellplacer/qplacer.cpp
    cellplacer/cellplacer.cpp
    cellplacer/densitybitmap.cpp
    cellplacer/netlistsplitter.cpp
    cellplacer/qlaplacer_private.cpp
    cellplacer/qlaplacer.cpp
    cellplacer/rowlegalizer.cpp
    celllib/cell.cpp
    celllib/pin.cpp
    celllib/module.cpp
    celllib/celllib.cpp
    techlib/techlib.cpp
    floorplan/floorplan.cpp
    floorplan/region.cpp
    floorplan/row.cpp
    design/design.cpp
    partitioner/fmpart.cpp
    import/liberty/libparser.cpp
    import/liberty/libreader.cpp
    import/liberty/libreaderimpl.cpp
    import/lef/lefparser.cpp
    import/lef/lefreaderimpl.cpp
    import/lef/lefreader.cpp
    import/verilog/veriloglexer.cpp
    import/verilog/verilogparser.cpp
    import/verilog/verilogreader.cpp
    export/svg/svgwriter.cpp
    export/dot/dotwriter.cpp
    export/def/defwriter.cpp
    export/verilog/verilogwriter.cpp
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
    ${PLATFORMSRC}
    )

target_include_directories(lunacore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(lunacore PRIVATE ${PYTHON_INCLUDE_DIRS})
target_link_libraries(lunacore Eigen3::Eigen ${PYTHON_LIBRARIES})
