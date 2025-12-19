vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
  OUT_SOURCE_PATH SOURCE_PATH
  REPO bartcich/bgfx.slang
  REF <TAG_OR_COMMIT> # UPDATE HERE
  SHA512 <SHASUM> # UPDATE HERE
)

vcpkg_cmake_configure(
  SOURCE_PATH ${SOURCE_PATH}
    OPTIONS -DBGFXSLANG_EXTERNAL_LIBS=ON
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "bgfx-slang")
vcpkg_copy_tools(TOOL_NAMES bgfx-slang-cmd AUTO_CLEAN)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
