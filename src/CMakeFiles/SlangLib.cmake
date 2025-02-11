include(FetchContent)

set(SLANG_VERSION 2025.4)

FetchContent_Declare(
    slang
    URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.tar.gz
    URL_HASH SHA256=bf124e82af5aa6d324e74b02a89a41c61d435493c5c4ac3be0b21e865038d915
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
