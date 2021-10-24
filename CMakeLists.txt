cmake_minimum_required(VERSION 3.16)

project(luna_pnr VERSION 0.1 HOMEPAGE_URL https://www.asicsforthemasses.com)

set(LUNA_INSTALL_PREFIX "/opt/lunapnr-${CMAKE_PROJECT_VERSION}")
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Packing)

enable_testing()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(contrib/lua-5.4.3)
add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(test)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/gui/lunapnr
    DESTINATION ${LUNA_INSTALL_PREFIX}/bin)