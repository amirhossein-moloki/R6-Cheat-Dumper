cmake_minimum_required(VERSION 3.20)
project(technical-suite VERSION 1.0 LANGUAGES CXX C)

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
