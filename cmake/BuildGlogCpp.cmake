# file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/glog/bin)
# file(MAKE_DIRECTORY ${PROJECT_BINARY_DIR}/glog/lib)

# set (_glog_CMake_BINDIR build/glog/bin CACHE STRING "Install Dir")
# set (_glog_CMake_INCLUDE_DIR build/glog CACHE STRING "Install Dir") 
# set (_glog_CMake_LIBDIR build/glog/lib CACHE STRING "Install Dir") 
# set (_glog_CMake_INSTALLDIR build/glog CACHE STRING "Install Dir")

set(BUILD_SHARED_LIBS OFF)
set(GTEST OFF)

add_subdirectory(thirdparty/glog)
