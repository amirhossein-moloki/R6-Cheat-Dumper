cmake_minimum_required(VERSION 3.20)
project(memdrv VERSION 1.0 LANGUAGES CXX C)

# Detect Windows SDK/WDK Path via Registry
execute_process(
    COMMAND cmd /c "reg query \"HKLM\\SOFTWARE\\Microsoft\\Windows Kits\\Installed Roots\" /v \"KitsRoot10\""
    OUTPUT_VARIABLE WDK_REG
    ERROR_QUIET
)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCES
    "memdrv/*.cpp"
    "memdrv/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/memdrv
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /GS- /W4 /Z7 /Qspectre- /MP
    )

    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/DRIVER /ENTRY:DriverEntry /SUBSYSTEM:NATIVE"
        VS_PLATFORM_TOOLSET "WindowsKernelModeDriver10.0" # Usually requires this toolset for drivers
    )
endif()
