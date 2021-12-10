cmake_minimum_required(VERSION 3.16)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(luna_pnr VERSION 0.1 HOMEPAGE_URL https://www.asicsforthemasses.com)


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
    endif()
endif(MSVC)


############################################################
## Main project
############################################################

enable_testing()

add_subdirectory(tools)
add_subdirectory(contrib/lua-5.4.3)
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

