file(REAL_PATH /usr/bin/c++ REAL_COMPILER_PATH)

execute_process(COMMAND ${REAL_COMPILER_PATH} --version
                COMMAND head -n 1
                OUTPUT_VARIABLE COMPILER_VERSION_STRING
                ERROR_QUIET)

if ("${COMPILER_VERSION_STRING}" STREQUAL "")
    set(COMPILER_VERSION_STRING "Compiler string N/A")
endif()

string(STRIP "${COMPILER_VERSION_STRING}" COMPILER_VERSION_STRING)

set(VERSION "const char* COMPILER_VERSION=\"'${COMPILER_VERSION_STRING}'\";")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version/compilerversion.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version/compilerversion.cpp VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version/compilerversion.cpp "${VERSION}")
endif()
