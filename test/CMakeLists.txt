cmake_minimum_required(VERSION 3.16)

project(test_suite)
enable_testing()

set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_EXPORT_COMPILE_COMMANDS=ON)

find_package(Boost QUIET COMPONENTS filesystem system unit_test_framework REQUIRED)

# collect all source files
file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

# get IIT cells
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/files/iit_stdcells/lib/README
    COMMENT "Retrieving IIT cells"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/files
    COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/files/get_cells.sh
    )

add_custom_target(iitcells DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/files/iit_stdcells/lib/README)

# test executable
add_executable(core_test ${SOURCES})
target_link_libraries(core_test lunacore stdc++fs ${Boost_LIBRARIES})
add_dependencies(core_test iitcells)

add_test(
    NAME core_test
    #COMMAND ${CMAKE_SOURCE_DIR}/build/tests/core
    COMMAND core_test
    #WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
