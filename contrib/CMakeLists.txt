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
        set(EIGEN_ARCHIVE S{CMAKE_CURRENT_SOURCE_DIR}/"eigen-3.4.0.tar.gz")
        file(DOWNLOAD "https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz" ${EIGEN_ARCHIVE} INACTIVITY_TIMEOUT 10)
        add_custom_command(
            OUTPUT eigen3/CMakeLists.txt
            COMMAND ${CMAKE_COMMAND} -E tar xzf ${EIGEN_ARCHIVE}
            DEPENDS ${EIGEN_ARCHIVE}
            COMMENT "Unpacking Eigen3 archive"
            VERBATIM
        )
    endif()        

    add_subdirectory(eigen3)
endif(MSVC)
