cmake_minimum_required(VERSION 3.4.3)

set(LLVM_ROOT "" CACHE PATH "Root of LLVM install.")

if(NOT EXISTS ${LLVM_ROOT}/include/llvm)
  message(FATAL_ERROR
    "LLVM_ROOT (${LLVM_ROO}) is invalid")
endif()

if(NOT EXISTS ${LLVM_ROOT}/bin/clang)
  message(FATAL_ERROR
    "clang not compiled with LLVM")
endif()

find_package(LLVM REQUIRED CONFIG)

list(APPEND CMAKE_MODULE_PATH "${LLVM_CMAKE_DIR}")
include(AddLLVM)

add_definitions(${LLVM_DEFINITIONS})
include_directories(${LLVM_INCLUDE_DIRS})

include_directories(${CMAKE_SOURCE_DIR}/include)

add_subdirectory(Analysis)
