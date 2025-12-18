project(bgfx-slang-cmd LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SRC *.cpp)
file(GLOB_RECURSE HEADERS *.h)

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE bgfx-slang)

# install slang dlls
if (NOT BGFXSLANG_EXTERNAL_LIBS)
add_custom_command(TARGET bgfx-slang-cmd POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
          $<TARGET_RUNTIME_DLLS:bgfx-slang-cmd>
          $<TARGET_FILE_DIR:bgfx-slang-cmd>
  COMMAND_EXPAND_LISTS
)
endif()

if (BGFXSLANG_INSTALL)
  install(TARGETS ${PROJECT_NAME} DESTINATION "${CMAKE_INSTALL_BINDIR}")
  install(FILES ${CMAKE_SOURCE_DIR}/cmake/bgfx-slang-toolUtils.cmake DESTINATION share/bgfx-slang)
endif()
