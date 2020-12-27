cmake_minimum_required(VERSION 3.11)

project(checked_cmd LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(EXTERN_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern)

# Provide path for scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
include(msvc_cplusplus)
include(warnings)
#include(MunkeiVersionFromGit)
include(clara)

## it spams the target list in CLion
include(CTestConfig)
include(CTest)
enable_testing()
add_library(checked_cmd INTERFACE)
add_library(${project}::checked_cmd ALIAS checked_cmd)

target_include_directories(checked_cmd
        INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
        $<INSTALL_INTERFACE:include>
        )

add_subdirectory(Tests)
