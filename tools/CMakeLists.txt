project(bgfx-slang-cmd LANGUAGES CXX VERSION 0.0.1)

file(GLOB_RECURSE SRC *.cpp)
file(GLOB_RECURSE HEADERS *.h)

add_executable(${PROJECT_NAME} ${SRC} ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE bgfx-slang)

# install slang dlls
file(GLOB dlls "${slang_SOURCE_DIR}/bin/*.dll")
file(INSTALL ${dlls} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)