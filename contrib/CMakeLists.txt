##  LunaPnR Source Code
##
##  SPDX-License-Identifier: GPL-3.0-only
##  SPDX-FileCopyrightText: 2022,2023 Niels Moseley <asicsforthemasses@gmail.com>
##  

cmake_minimum_required(VERSION 3.16)

############################################################
## Install Eigen3 for Windows
############################################################

if (MSVC)
    if (NOT EXISTS eigen3/CMakeLists.txt)

        message(STATUS "retrieving Eigen3...")
        
        set(EIGEN_ARCHIVE_FILE "eigen-3.4.0.tar.gz")        
        set(EIGEN_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/${EIGEN_ARCHIVE_FILE}")

        set(EIGEN_ARCHIVE_DIR "eigen-3.4.0")
        set(EIGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/eigen3")

        message(STATUS "  location: ${EIGEN_ARCHIVE}")
        file(DOWNLOAD "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz" ${EIGEN_ARCHIVE} INACTIVITY_TIMEOUT 10)
        file(ARCHIVE_EXTRACT INPUT ${EIGEN_ARCHIVE} DESTINATION ${CMAKE_CURRENT_SOURCE_DIR})
        file(RENAME ${EIGEN_ARCHIVE_DIR} ${EIGEN_DIR})
    endif()        

    add_subdirectory(eigen3)
endif(MSVC)
