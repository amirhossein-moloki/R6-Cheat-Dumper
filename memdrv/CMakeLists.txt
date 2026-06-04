cmake_minimum_required(VERSION 3.20)
project(memdrv VERSION 1.0 LANGUAGES CXX C)

# ============================================
# 1. Environment Detection (WDK 10.0.28000.0)
# ============================================
set(KITS_ROOT "C:/Program Files (x86)/Windows Kits/10")
set(SDK_VERSION "10.0.28000.0")

message(STATUS "Configuring Kernel Driver with WDK ${SDK_VERSION}")

# ============================================
# 2. Source Files
# ============================================
file(GLOB_RECURSE SOURCES
    "memdrv/*.cpp"
    "memdrv/*.hpp"
)

# ============================================
# 3. Target Definition
# ============================================
add_executable(${PROJECT_NAME} ${SOURCES})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".sys")

# ============================================
# 4. Include Directories
# ============================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/memdrv
    "${KITS_ROOT}/Include/${SDK_VERSION}/km"
    "${KITS_ROOT}/Include/${SDK_VERSION}/shared"
)

# ============================================
# 5. Compiler Options
# ============================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /kernel          # Kernel mode
        /GS-             # Disable Buffer Security Check
        /Qspectre-       # Disable Spectre
        /guard:cf-       # Disable Control Flow Guard
        /W1              # Warning level 1
        /Z7              # Full symbolic debugging information
        /MP              # Multi-processor compilation
    )

    # ============================================
    # 6. Linker Options
    # ============================================
    target_link_options(${PROJECT_NAME} PRIVATE
        /DRIVER             # Driver mode
        /ENTRY:DriverEntry  # Driver entry point
        /SUBSYSTEM:NATIVE   # Native subsystem
        /MANIFEST:NO        # No manifest
        /GUARD:NO           # No guard
    )

    # ============================================
    # 7. Library Dependencies
    # ============================================
    target_link_directories(${PROJECT_NAME} PRIVATE
        "${KITS_ROOT}/Lib/${SDK_VERSION}/km/x64"
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ntoskrnl.lib
        hal.lib
        wdmsec.lib
    )

    # ============================================
    # 8. Platform Properties
    # ============================================
    set_target_properties(${PROJECT_NAME} PROPERTIES
        VS_PLATFORM_TOOLSET "v145"
    )
endif()
