include(FetchContent)

set(SLANG_VERSION 2025.22.1)

FetchContent_Declare(
    slang
    URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.zip
    URL_HASH SHA512=de073b816df5c1d5fe1c0580dd11770eb9e03188922e58997a22f6fe910434e2efdbbfa3908c030011b6e704a89db58454f7100c4f26c9f3175b329f3b79ead8
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(slang)

set(SLANG_INCLUDE_DIR "${slang_SOURCE_DIR}/include")
set(SLANG_BIN_DIR "${slang_SOURCE_DIR}/bin")
set(SLANG_LIB_DIR "${slang_SOURCE_DIR}/lib")


add_library(slang::slang SHARED IMPORTED GLOBAL)

set_property(TARGET slang::slang APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)

if (WIN32) 
    set(SLANG_LIB "${SLANG_LIB_DIR}/slang.lib")
    set(SLANG_BIN "${SLANG_BIN_DIR}/slang.dll;${SLANG_BIN_DIR}/slang-compiler.dll")
    
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
