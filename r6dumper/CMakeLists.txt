cmake_minimum_required(VERSION 3.20)
project(r6dumper VERSION 1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect Windows SDK Path via Registry
execute_process(
    COMMAND cmd /c "reg query \"HKLM\\SOFTWARE\\Microsoft\\Microsoft SDKs\\Windows\\v10.0\" /v \"InstallationFolder\""
    OUTPUT_VARIABLE WINDOWS_SDK_REG
    ERROR_QUIET
)

if(MSVC)
    add_compile_options(/Qspectre-)
endif()

file(GLOB SOURCES
    "src/*.cpp"
    "src/*.hpp"
    "src/*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /MP /volatile:iso)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:CONSOLE"
        VS_PLATFORM_TOOLSET "v145"
    )
endif()

# Copy offsets.db to the output directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/src/offsets.db"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>/offsets.db"
)
