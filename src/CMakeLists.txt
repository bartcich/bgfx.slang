option(BGFXSLANG_EXTERNAL_LIBS "Use external library instead of bundled" OFF)
option(BGFXSLANG_INSTALL "Install the library" ON)

file(GLOB_RECURSE SRC BgfxSlang/*.cpp)
file(GLOB_RECURSE HEADERS BgfxSlang/*.h)

add_library(${PROJECT_NAME} STATIC ${SRC} ${HEADERS})

target_include_directories(${PROJECT_NAME} PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if (BGFXSLANG_EXTERNAL_LIBS)
    find_library(SLANG_LIB slang)
    find_path(SLANG_INCLUDE_DIR NAMES slang.h PATH_SUFFIXES include)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${SLANG_LIB})
    target_include_directories(${PROJECT_NAME} PUBLIC ${SLANG_INCLUDE_DIR})

    find_package(spirv_cross_core CONFIG REQUIRED)
    find_package(spirv_cross_glsl CONFIG REQUIRED)
else()
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/SlangLib.cmake)
    include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/SpirvCrossLib.cmake)
    target_link_libraries(${PROJECT_NAME} PUBLIC slang::slang)
endif()
    
target_link_libraries(${PROJECT_NAME} PUBLIC spirv-cross-core spirv-cross-glsl)

if (BGFXSLANG_INSTALL)

    install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}_targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")

    install(DIRECTORY BgfxSlang/ DESTINATION include/bgfx-slang FILES_MATCHING PATTERN "*.h")

    install(EXPORT ${PROJECT_NAME}_targets
        FILE ${PROJECT_NAME}-targets.cmake
        NAMESPACE bgfx-slang::
        DESTINATION share/bgfx-slang)

    install(FILES "${CMAKE_SOURCE_DIR}/cmake/bgfx-slang-config.cmake" DESTINATION share/bgfx-slang)

endif()