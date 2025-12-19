include(FetchContent)

set(SLANG_VERSION 2025.22.1)

message(STATUS "Fetching Slang v${CMAKE_HOST_SYSTEM_PROCESSOR} binaries...")

if (WIN32)
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "ARM64")
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-aarch64.zip
            URL_HASH SHA512=7aa4c9b8652818c79fd63c8e23e3f2cfbbaa2fe4ec00cb435364fcb8c8a557beb080f435da8f10710bc1a6ccdd5a2177db124daec051bc34350b56a0aad81d85
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    else()
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.zip
            URL_HASH SHA512=de073b816df5c1d5fe1c0580dd11770eb9e03188922e58997a22f6fe910434e2efdbbfa3908c030011b6e704a89db58454f7100c4f26c9f3175b329f3b79ead8
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    endif()
elseif(APPLE)
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-macos-aarch64.zip
            URL_HASH SHA512=5e8bba5784ad5d30e9b92a5754ad7687ada8899aa55516c0cad7a5d8c09fb2cba83a09c9bd06911c8fe7d464ebece8d72376f810881d5c73c4b4a3246b020a7b
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    else()
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-macos-x86_64.zip
            URL_HASH SHA512=f74a4c94e6b84f1359d680c1a44cf441f4043158efec0a835379227ade2f72557efabdce6518054bfd1036a85c8a7355918b1a70179ccd06bf3fbb8c49c6fcc5
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        endif()
else()
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-linux-aarch64.zip
            URL_HASH SHA512=56d0f975c629e53c45e9d075fe71630ba488f6ad2da0cfcea27e28c726607152d451b7b3dacf10248e0c80535499c4c5c9ede6909c10c29c75a40bf553a9af64
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    else()
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-linux-x86_64.zip
            URL_HASH SHA512=d9f398cfe902b948290bad9e158e82baea0c6bd942e9cf9ae272546167c63a289fe1ef9a6048b658f1c9ecd51cbbcd187675da87f39ca05ad289eaa93a7fd30b
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    endif()
endif()

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
