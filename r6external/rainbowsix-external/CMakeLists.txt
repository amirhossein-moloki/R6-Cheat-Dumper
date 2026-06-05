cmake_minimum_required(VERSION 3.20)
project(rainbowsix-external VERSION 1.0 LANGUAGES CXX C)

# ============================================
# 0. Global Options
# ============================================
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ============================================
# 1. Environment Detection
# ============================================
set(KITS_ROOT "C:/Program Files (x86)/Windows Kits/10")
set(SDK_VERSION "10.0.26100.0")

# ============================================
# 2. External Dependencies
# ============================================
include(FetchContent)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(nlohmann_json spdlog)

# ============================================
# 3. Source Files
# ============================================
file(GLOB_RECURSE SOURCES
    "core/*.cpp"
    "core/*.hpp"
    "driver/*.cpp"
    "driver/*.hpp"
    "features/*.cpp"
    "features/*.hpp"
    "game/*.cpp"
    "game/*.h"
    "overlay/*.cpp"
    "overlay/*.hpp"
    "overlay/imgui/*.cpp"
    "overlay/imgui/*.h"
    "overlay/input/*.cpp"
    "overlay/input/*.hpp"
    "overlay/minhook/*.c"
    "overlay/minhook/*.h"
    "overlay/minhook/hde/*.c"
    "overlay/minhook/hde/*.h"
    "overlay/renderer/*.cpp"
    "overlay/renderer/*.hpp"
    "util/*.cpp"
    "util/*.hpp"
    "util/math/*.cpp"
    "util/math/*.hpp"
    "config.cpp"
    "config.hpp"
    "globals.cpp"
    "globals.hpp"
    "main.cpp"
    "offsets.hpp"
)

# ============================================
# 4. Executable Target
# ============================================
add_executable(${PROJECT_NAME} ${SOURCES})

# ============================================
# 5. Include Directories
# ============================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/features
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay/minhook
    "${KITS_ROOT}/Include/${SDK_VERSION}/um"
    "${KITS_ROOT}/Include/${SDK_VERSION}/shared"
)

# ============================================
# 6. Link Libraries
# ============================================
target_link_libraries(${PROJECT_NAME}
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    d3d11
    dxgi
    d3dcompiler
    winmm
    user32
    gdi32
)

# ============================================
# 7. Compiler & Linker Options
# ============================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /MP
        /std:c++17
        /volatile:iso
        /Qspectre-
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        VS_PLATFORM_TOOLSET "v143"
    )
endif()

# ============================================
# 8. Static Analysis
# ============================================
find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
if(CLANG_TIDY_EXE)
    set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=-*,readability-*,bugprone-*,performance-*,portability-*,modernize-*;--warnings-as-errors=*")
    message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
else()
    message(STATUS "clang-tidy not found.")
endif()
