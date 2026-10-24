# These modules are loaded at runtime and are not discoverable as DLL imports.
file(GLOB STANDARD_MODULE_DIRS "${SLANG_RUNTIME_DIR}/slang-standard-module-*")
foreach(MODULE_DIR IN LISTS STANDARD_MODULE_DIRS)
  file(COPY "${MODULE_DIR}" DESTINATION "${OUTPUT_DIR}")
endforeach()
file(COPY "${SLANG_RUNTIME_DIR}/slang.slang" "${SLANG_RUNTIME_DIR}/gfx.slang"
  DESTINATION "${OUTPUT_DIR}"
)
