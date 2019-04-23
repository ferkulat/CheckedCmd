cmake_minimum_required(VERSION 3.11)

set (MY_PROJECTNAME checked_cmd)

project(${MY_PROJECTNAME} LANGUAGES CXX)

set(EXTERN_CLARA_TAG v1.1.5)

set(EXTERN_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern)
set(EXTERN_CLARA_HEADER_DIR ${EXTERN_DEPENDENCIES_DIR}/clara)

# Provide path for scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")

include(warnings)
#include(MunkeiVersionFromGit)
include(clara)

## it spams the target list in CLion
include(CTestConfig)
include(CTest)
enable_testing()

if( CMAKE_CXX_COMPILER_ID MATCHES "MSVC" )
    #https://devblogs.microsoft.com/cppblog/msvc-now-correctly-reports-__cplusplus/
    #Clara checks for this value to enable support for std::optional
    set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus" )
endif()

include_directories(include)

set(CMAKE_CXX_STANDARD 17)
add_subdirectory(Tests)
set(SOURCE_FILES main.cpp )
add_executable(${MY_PROJECTNAME} ${SOURCE_FILES})
