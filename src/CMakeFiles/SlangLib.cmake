include(FetchContent)

set(SLANG_VERSION 2025.6.3)

FetchContent_Declare(
    slang
    URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.zip
    URL_HASH SHA512=95a647ae4c446cb8227d1bbddc013549a41b22c8de561c5938f5e56182de86e8108852a0f066bdc16bfcb1339be048907410fc1ece5939d556d58c95999667ed
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(Slang)

set(SLANG_INCLUDE_DIR "${slang_SOURCE_DIR}/include")
set(SLANG_BIN_DIR "${slang_SOURCE_DIR}/bin")
set(SLANG_LIB_DIR "${slang_SOURCE_DIR}/lib")


add_library(slang::slang SHARED IMPORTED)

set_property(TARGET slang::slang APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)

if (WIN32) 
    set(SLANG_LIB "${SLANG_LIB_DIR}/slang.lib")
    set(SLANG_BIN "${SLANG_BIN_DIR}/slang.dll")
    
    set_target_properties(slang::slang PROPERTIES
        IMPORTED_LOCATION            "${SLANG_BIN}"
        IMPORTED_IMPLIB              "${SLANG_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
    )
elseif (APPLE)
    set(SLANG_LIB "${SLANG_LIB_DIR}/libslang.dylib")
    
    set_target_properties(slang::slang PROPERTIES
        IMPORTED_LOCATION            "${SLANG_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
    )
 else() 
    set(SLANG_LIB "${SLANG_LIB_DIR}/libslang.so")
    
    set_target_properties(slang::slang PROPERTIES
        IMPORTED_LOCATION            "${SLANG_LIB}"
        INTERFACE_INCLUDE_DIRECTORIES "${SLANG_INCLUDE_DIR}"
    )
endif()
