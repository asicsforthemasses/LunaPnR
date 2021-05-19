project(lunacore)

set(CMAKE_CXX_STANDARD 17)

option(UseCPPCHECK "UseCPPCHECK" ON)
set(CMAKE_CXX_CPPCHECK "cppcheck")

# CPPCHECK static analysis support
if (CMAKE_CXX_CPPCHECK AND UseCPPCHECK)
find_program(CMAKE_CXX_CPPCHECK NAMES cppcheck)
    list(
        APPEND CMAKE_CXX_CPPCHECK 
            "--enable=warning"
            "--inconclusive"
            "--force" 
            "--inline-suppr"
            "--suppressions-list=${CMAKE_CURRENT_SOURCE_DIR}/cppcheck/CppCheckSuppressions.txt"
    )
endif()

add_library(lunacore 
    common/logging.cpp
    import/verilog/veriloglexer.cpp
    import/verilog/verilogparser.cpp)

target_include_directories(lunacore PRIVATE common)
