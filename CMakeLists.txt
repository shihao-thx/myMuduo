cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

project(muduo LANGUAGES C CXX)

option(USE_SHARED_LIB "Compile sources into a shared library." OFF)

if (NOT CMAKE_BUILD_TYPE)
  set (CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

include(cmake/BuildGlogCpp.cmake)

file(GLOB_RECURSE MUDUO_SRC_FILES "src/net/*.cc" "src/net/*.h")
file(GLOB_RECURSE MUDUO_UTIL_FILES "src/util/*.h" "src/util/*.cc")
add_library(muduo ${MUDUO_SRC_FILES} ${MUDUO_UTIL_FILES})
target_link_libraries(muduo STATIC glog gflags unwind)
target_include_directories(muduo PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}/thirdparty/glog)

