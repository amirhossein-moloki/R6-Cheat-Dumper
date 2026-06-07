cmake_minimum_required(VERSION 3.20)
project(r6dumper VERSION 1.0 LANGUAGES CXX C)

# ============================================
# 1. Environment Detection
# ============================================
if(DEFINED ENV{WindowsSdkDir})
    file(TO_CMAKE_PATH "$ENV{WindowsSdkDir}" KITS_ROOT)
else()
    set(KITS_ROOT "C:/Program Files (x86)/Windows Kits/10")
endif()

if(NOT DEFINED SDK_VERSION)
    if(DEFINED ENV{WindowsSDKVersion})
        string(REGEX REPLACE "\\\\$" "" SDK_VERSION_ENV "$ENV{WindowsSDKVersion}")
        set(SDK_VERSION ${SDK_VERSION_ENV})
    else()
        set(SDK_VERSION "10.0.28000.0")
    endif()
endif()

# ============================================
# 2. External Dependencies
# ============================================
include(FetchContent)

FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.12.0
)
FetchContent_MakeAvailable(spdlog)

# ============================================
# 3. Source Files
# ============================================
file(GLOB SOURCES
    "src/*.cpp"
    "src/*.hpp"
    "src/*.h"
)

# ============================================
# 4. Target Definition
# ============================================
add_executable(${PROJECT_NAME} ${SOURCES})

# ============================================
# 5. Include Directories
# ============================================
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    "${KITS_ROOT}/Include/${SDK_VERSION}/um"
    "${KITS_ROOT}/Include/${SDK_VERSION}/shared"
)

# ============================================
# 6. Link Libraries
# ============================================
target_link_libraries(${PROJECT_NAME}
    spdlog::spdlog
)

# ============================================
# 7. Compiler & Linker Options
# ============================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /MP
        /volatile:iso
        /Qspectre-
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        VS_PLATFORM_TOOLSET "v143"
    )
endif()

# ============================================
# 6. Post-Build Actions
# ============================================
# Copy offsets.db to the output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/src/offsets.db"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/offsets.db"
)
