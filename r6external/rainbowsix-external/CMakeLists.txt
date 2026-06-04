cmake_minimum_required(VERSION 3.20)
project(rainbowsix-external VERSION 1.0 LANGUAGES CXX C)

# ============================================
# 1. Compiler and Platform Settings
# ============================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Disable Spectre Mitigation to avoid MSB8040
if(MSVC)
    add_compile_options(/Qspectre-)
endif()

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
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/overlay/minhook
)

# ============================================
# 5. Link Libraries
# ============================================
target_link_libraries(${PROJECT_NAME}
    d3d11
    dxgi
    d3dcompiler
    winmm
)

# ============================================
# 6. Compiler & Linker Options
# ============================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /MP
        /volatile:iso
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
    )
endif()
