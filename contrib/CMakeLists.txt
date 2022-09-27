cmake_minimum_required(VERSION 3.16)

find_package(Git REQUIRED)

# automatically install tinysvgpp if the user didn't do this..
if(NOT EXISTS tinysvgpp/CMakeLists.txt)
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive contrib/tinysvgpp
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
endif()

