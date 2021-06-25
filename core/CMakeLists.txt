project(lunacore)

set(CMAKE_CXX_STANDARD 17)

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
            "-checks=*,-modernize-use-trailing-return-type,-google-build-using-namespace,-cppcoreguidelines-pro-type-vararg,-hicpp-vararg"
    )
endif()

add_library(lunacore 
    common/logging.cpp
    common/dbtypes.cpp
    netlist/design.cpp
    netlist/net.cpp
    netlist/instance.cpp
    netlist/netlist.cpp
    cellplacer/cellplacer.cpp
    celllib/pin.cpp
    celllib/celllib.cpp
    techlib/techlib.cpp
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
    export/dot/dotwriter.cpp
    )

target_include_directories(lunacore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
