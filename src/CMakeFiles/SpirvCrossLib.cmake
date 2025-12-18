include(FetchContent)

set(SPIRV_CROSS_VERSION vulkan-sdk-1.4.335.0)


FetchContent_Declare(
  spirv-cross
  GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross
  GIT_TAG ${SPIRV_CROSS_VERSION}
)

FetchContent_MakeAvailable(spirv-cross)

