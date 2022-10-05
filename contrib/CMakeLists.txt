cmake_minimum_required(VERSION 3.16)

find_package(Git REQUIRED)

# automatically install tinysvgpp if the user didn't do this..
if(NOT EXISTS tinysvgpp/CMakeLists.txt)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive contrib/tinysvgpp
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
endif()

############################################################
## Install Eigen3 for Windows
############################################################

if (MSVC)
    if (NOT EXISTS eigen3/CMakeLists.txt)

        message(STATUS "retrieving Eigen3...")
        set(EIGEN_ARCHIVE "${CMAKE_CURRENT_SOURCE_DIR}/eigen-3.4.0.tar.gz")
        set(EIGEN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/eigen3")
        message(STATUS "  location: ${EIGEN_ARCHIVE}")
        file(DOWNLOAD "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz" ${EIGEN_ARCHIVE} INACTIVITY_TIMEOUT 10)
        file(ARCHIVE_EXTRACT ${EIGEN_ARCHIVE} DESTINATION ${EIGEN_DIR})
    endif()        

    add_subdirectory(eigen3)
endif(MSVC)
