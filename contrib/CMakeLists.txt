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
    message(STATUS "retrieving Eigen3...")
    set(EIGEN_ARCHIVE "eigen-3.4.0.tar.gz")
    file(DOWNLOAD "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz" ${EIGEN_ARCHIVE} INACTIVITY_TIMEOUT 10)
    file(ARCHIVE_EXTRACT INPUT ${EIGEN_ARCHIVE} DESTINATION "eigen3")
    add_subdirectory(eigen3)
endif(MSVC)
