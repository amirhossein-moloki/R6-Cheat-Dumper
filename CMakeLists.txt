cmake_minimum_required(VERSION 3.20)
project(technical-suite VERSION 1.0)

# ============================================
# Root CMake configuration for the Technical Suite
# ============================================

# Component 1: Kernel Driver
add_subdirectory(memdrv)

# Component 2: Offset Dumper
add_subdirectory(r6dumper)

# Component 3: External Memory Interface
add_subdirectory(r6external/rainbowsix-external)
