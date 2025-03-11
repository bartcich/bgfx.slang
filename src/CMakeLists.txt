project(bgfx-slang-lib VERSION 0.0.1 LANGUAGES CXX)
option(BGFXSLANG_EXTERNAL_LIBS "Use external library instead of bundled" OFF)

file(GLOB_RECURSE SRC BgfxSlang/*.cpp)
file(GLOB_RECURSE HEADERS BgfxSlang/*.h)

add_library(${PROJECT_NAME} STATIC ${SRC} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if (BGFXSLANG_EXTERNAL_LIBS)
    find_package(slang REQUIRED)
    find_package(spirv-cross REQUIRED)
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/SlangLib.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/SpirvCrossLib.cmake)
endif()

target_link_libraries(${PROJECT_NAME} PUBLIC slang::slang)
target_link_libraries(${PROJECT_NAME} PUBLIC spirv-cross-core spirv-cross-glsl)

