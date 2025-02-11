project(bgfx-slang-lib VERSION 0.0.1 LANGUAGES CXX)
option(BGFXSLANG_EXTERNAL_LIBS "Use external library instead of bundled" OFF)

file(GLOB_RECURSE SRC BgfxSlang/*.cpp)
file(GLOB_RECURSE HEADERS BgfxSlang/*.h)

add_library(${PROJECT_NAME} STATIC ${SRC} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC slang::slang)

include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/SlangLib.cmake)
