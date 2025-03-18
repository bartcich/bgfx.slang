include(CMakeFindDependencyMacro)
find_dependency(spirv_cross_core CONFIG REQUIRED)
find_dependency(spirv_cross_glsl CONFIG REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/bgfx-slang-targets.cmake)