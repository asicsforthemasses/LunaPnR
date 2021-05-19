cmake_minimum_required(VERSION 3.16)

project(luna_pnr)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(core)
add_subdirectory(gui)
add_subdirectory(test)

