cmake_minimum_required(VERSION 3.11)

project(checked_cmd LANGUAGES CXX)

set(EXTERN_DEPENDENCIES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern)

# Provide path for scripts
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMake")
include(msvc_cplusplus)
include(warnings)
include(catch2)
#include(MunkeiVersionFromGit)
include(clara)

## it spams the target list in CLion
include(CTestConfig)
include(CTest)
enable_testing()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
add_subdirectory(Tests EXCLUDE_FROM_ALL)
