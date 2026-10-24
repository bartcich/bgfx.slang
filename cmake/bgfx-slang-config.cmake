include(CMakeFindDependencyMacro)
find_dependency(slang CONFIG REQUIRED)
find_dependency(spirv_cross_core CONFIG REQUIRED)
find_dependency(spirv_cross_glsl CONFIG REQUIRED)
find_dependency(spirv_cross_msl CONFIG REQUIRED)

include(${CMAKE_CURRENT_LIST_DIR}/bgfx-slang-targets.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/bgfx-slang-runtime.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/bgfx-slang-toolUtils.cmake)
