include(FetchContent)

set(SLANG_VERSION 2026.20)

message(STATUS "Fetching Slang v${CMAKE_HOST_SYSTEM_PROCESSOR} binaries...")

if (WIN32)
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "ARM64")
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-aarch64.zip
            URL_HASH SHA256=4f18a7e76a59969bc9bf6ea596bd9e496a3c689af173990ae028b18f03e1ac1a
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    else()
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-windows-x86_64.zip
            URL_HASH SHA256=e6dd32bbddf1720ebb4e679d55e9635ce792865a5bd84f3d2cd9af28647c0174
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    endif()
elseif(APPLE)
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-macos-aarch64.zip
            URL_HASH SHA256=ede1865bf086442c8786c5b7c0470ce5f4fd4f032eefe881356726ff9c445f40
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    else()
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-macos-x86_64.zip
            URL_HASH SHA256=a71d4a499d08b6178325a697a5a5c9fddc9eee5914daaf195451b283189e96ce
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
        endif()
else()
    if (${CMAKE_HOST_SYSTEM_PROCESSOR} STREQUAL "arm64")
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-linux-aarch64.zip
            URL_HASH SHA256=1901efe981900908faba8109bcffaf4df48068bb0e5e42ffadb908af525e7ed8
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    else()
        FetchContent_Declare(
            slang
            URL https://github.com/shader-slang/slang/releases/download/v${SLANG_VERSION}/slang-${SLANG_VERSION}-linux-x86_64.zip
            URL_HASH SHA256=73681e314bd48c1396daf963a66647724519b152886961d8ca3b1b7e477cf640
            DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        )
    endif()
endif()

FetchContent_MakeAvailable(slang)

# Use the package's compiler target, including its platform-specific library name.
find_package(slang CONFIG REQUIRED GLOBAL
    PATHS "${slang_SOURCE_DIR}/cmake" "${slang_SOURCE_DIR}/lib/cmake/slang"
    NO_DEFAULT_PATH
)
