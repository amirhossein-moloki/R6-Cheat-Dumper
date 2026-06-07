cmake_minimum_required(VERSION 3.20)
project(memdrv VERSION 1.0 LANGUAGES CXX C)

# ============================================
# 1. Environment Detection
# ============================================
if(DEFINED ENV{WDKContentRoot})
    file(TO_CMAKE_PATH "$ENV{WDKContentRoot}" KITS_ROOT)
elseif(DEFINED ENV{WindowsSdkDir})
    file(TO_CMAKE_PATH "$ENV{WindowsSdkDir}" KITS_ROOT)
else()
    set(KITS_ROOT "C:/Program Files (x86)/Windows Kits/10")
endif()

# Attempt to find Windows SDK/WDK version if not set
if(NOT DEFINED SDK_VERSION)
    if(DEFINED ENV{WindowsSDKVersion})
        string(REGEX REPLACE "\\\\$" "" SDK_VERSION_ENV "$ENV{WindowsSDKVersion}")
        set(SDK_VERSION ${SDK_VERSION_ENV})
    elseif(DEFINED ENV{WDK_VERSION})
        set(SDK_VERSION "$ENV{WDK_VERSION}")
    else()
        set(SDK_VERSION "10.0.28000.0") # Fallback to a common version
    endif()
endif()

message(STATUS "Configuring Kernel Driver with WDK ${SDK_VERSION} at ${KITS_ROOT}")

# Check if WDK headers exist, if not make component optional
if(NOT EXISTS "${KITS_ROOT}/Include/${SDK_VERSION}/km/ntifs.h")
    message(WARNING "WDK headers not found at ${KITS_ROOT}/Include/${SDK_VERSION}/km/. memdrv will not be built.")
    return()
endif()

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
        /wd4117          # Suppress warning C4117: macro name '_KERNEL_MODE' is reserved
        /EHs-            # Disable C++ exception handling
        /EHc-
    )

    target_compile_definitions(${PROJECT_NAME} PRIVATE _AMD64_)

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

endif()
