cmake_minimum_required(VERSION 3.20)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# Read version from file
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" VERSION_RAW)
string(STRIP "${VERSION_RAW}" VERSION_STR)

project(technical-suite VERSION ${VERSION_STR} LANGUAGES CXX C)

# ============================================
# Global Settings
# ============================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Component 1: Kernel Driver
add_subdirectory(memdrv)

# Component 2: Offset Dumper
add_subdirectory(r6dumper)

# Component 3: External Memory Interface
add_subdirectory(r6external/rainbowsix-external)

# Testing
enable_testing()
add_subdirectory(tests)
