cmake_minimum_required(VERSION 3.20)
project(rainbowsix-external VERSION 1.0 LANGUAGES CXX C)

# ============================================
# 1. Environment Detection
# ============================================
set(KITS_ROOT "C:/Program Files (x86)/Windows Kits/10")
set(SDK_VERSION "10.0.28000.0")

# ============================================
# 2. Source Files
# ============================================
file(GLOB_RECURSE SOURCES
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
# 3. Executable Target
# ============================================
add_executable(${PROJECT_NAME} ${SOURCES})

# ============================================
# 4. Include Directories
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
# 5. Link Libraries
# ============================================
target_link_libraries(${PROJECT_NAME}
    d3d11
    dxgi
    d3dcompiler
    winmm
    user32
    gdi32
)

# ============================================
# 6. Compiler & Linker Options
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
        VS_PLATFORM_TOOLSET "v145"
    )
endif()
