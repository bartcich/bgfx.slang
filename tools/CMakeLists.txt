project(bgfx-slang-cmd LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SRC *.cpp)
file(GLOB_RECURSE HEADERS *.h)

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE bgfx-slang)

bgfx_slang_copy_runtime(bgfx-slang-cmd)

if (BGFXSLANG_INSTALL)
  install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
  install(FILES ${CMAKE_SOURCE_DIR}/cmake/bgfx-slang-toolUtils.cmake DESTINATION share/bgfx-slang)
endif()
