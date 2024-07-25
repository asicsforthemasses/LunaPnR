##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022,2024 Niels Moseley <asicsforthemasses@gmail.com>
##

cmake_minimum_required(VERSION 3.20)

## Make Debian/Ubuntu happy: this way the executable is not detected as a shared lib :-/
set(CMAKE_POSITION_INDEPENDENT_CODE OFF)

project(test_suite)

find_package(Boost QUIET COMPONENTS filesystem system unit_test_framework REQUIRED)

###########################################################
# TinyTestPP frame work
# linking name: tinytestpp
###########################################################

option(UseTinyTestPP "Use TinyTestPP test framework" ON)

if(UseTinyTestPP)
    CPMAddPackage("bb:trcwm/tinytestpp#main")
    target_compile_definitions(tinytestpp PRIVATE -DUSE_TINYTESTPP_MAIN=OFF -DBUILD_DOC=OFF)  # disable the main, conflicts with BOOST::TEST
endif(UseTinyTestPP)


###########################################################
# collect all source files
###########################################################

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true *.h *.cpp)

###########################################################
# get IIT cells
###########################################################

if(WIN32)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/files/iit_stdcells/lib/README
        COMMENT "Retrieving IIT cells"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/files
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/files/get_cells.bat
        )
endif(WIN32)

if(UNIX)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/../files/iit_stdcells/lib/README
        COMMENT "Retrieving IIT cells"
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../files
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../files/get_cells.sh
        )
endif(UNIX)

add_custom_target(iitcells DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../files/iit_stdcells/lib/README)

# test executable
add_executable(core_test ${SOURCES})
target_link_libraries(core_test lunacore ${Boost_LIBRARIES} tinytestpp)
target_include_directories(core_test PRIVATE ../../core/include)
add_dependencies(core_test iitcells)

# ignore clang tidy because it barfs on BOOST_CHECK macros
set_target_properties(core_test PROPERTIES CXX_CLANG_TIDY "")

add_test(
    NAME core_test
    COMMAND $<TARGET_FILE:core_test>
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    )
