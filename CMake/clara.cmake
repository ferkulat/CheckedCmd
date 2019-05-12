cmake_minimum_required(VERSION 3.11 FATAL_ERROR)
set(EXTERN_CLARA_TAG v1.1.5)
set(EXTERN_CLARA_HEADER_DIR ${EXTERN_DEPENDENCIES_DIR}/clara)

include(FetchContent)
# ------------------------------------------------------------------------------
# A modern, C++-native, header-only, test framework for unit-tests,
# TDD and BDD - using C++11, C++14, C++17 and later
FetchContent_Declare(
  extern_clara
  DOWNLOAD_NO_EXTRACT true
  DOWNLOAD URL        https://github.com/catchorg/Clara/releases/download/${EXTERN_CLARA_TAG}/clara.hpp
  DOWNLOAD_DIR        ${EXTERN_CLARA_HEADER_DIR}
)

FetchContent_GetProperties(extern_clara)
if(NOT extern_clara_POPULATED)
  FetchContent_Populate(extern_clara)
endif()
include_directories(${EXTERN_CLARA_HEADER_DIR})