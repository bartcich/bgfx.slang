cmake_minimum_required(VERSION 3.24)
set(CMAKE_CXX_STANDARD 20)
project(bgfx-slang CXX)
include(GNUInstallDirs)

add_subdirectory(src)
include(cmake/bgfx-slang-runtime.cmake)
add_subdirectory(tools)
