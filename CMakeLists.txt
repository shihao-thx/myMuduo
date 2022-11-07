cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(muduo LANGUAGES C CXX)

option(USE_SHARED_LIB "Compile sources into a shared library." OFF)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

file(GLOB_RECURSE MUDUO_SRC_FILES "muduo/net/*.cc" "muduo/net/*.h")
file(GLOB_RECURSE MUDUO_UTIL_FILES "muduo/util/*.h" "muduo/util/*.cc")
add_library(muduo ${MUDUO_SRC_FILES} ${MUDUO_UTIL_FILES})
target_include_directories(muduo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

