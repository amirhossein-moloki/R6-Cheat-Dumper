# Developer Onboarding & Workflow Guide

## Environment Setup

### Prerequisites
*   **Operating System:** Windows 10/11 (x64)
*   **Compiler:** Visual Studio 2022 (v143 toolset) or later.
*   **Build System:** CMake 3.20+
*   **Windows SDK:** 10.0.22621.0 or later.
*   **WDK:** Windows Driver Kit matching your Windows version.

### Initial Setup
1.  Clone the repository:
    ```bash
    git clone <repository_url>
    cd technical-suite
    ```
2.  Initialize submodules (if any):
    ```bash
    git submodule update --init --recursive
    ```

## Building the Project

### Using CMake (Recommended)
We provide a helper script for building with CMake:
```cmd
build_cmake.bat
```
This will configure the project, download dependencies (nlohmann_json, spdlog, googletest), and build all components in Release mode.

### Output Binaries
*   `r6external`: `build/r6external/rainbowsix-external/Release/rainbowsix-external.exe`
*   `r6dumper`: `build/r6dumper/Release/r6dumper.exe`
*   `memdrv`: `build/memdrv/memdrv/Release/memdrv.sys`

## Developer Workflow

### Coding Standards
*   Follow the `.clang-format` specification.
*   **Classes:** PascalCase (e.g., `MemoryService`)
*   **Functions/Variables:** snake_case (e.g., `read_memory`)
*   **Files:** snake_case (e.g., `config_service.cpp`)

### Adding New Features
1.  Create a new service or feature class.
2.  Register it in `CheatContext` if it's a shared resource.
3.  Use the `Logger` for all output (avoid `std::cout`).
4.  Add unit tests in the `tests/` directory.

### Testing
Run tests using CTest after building:
```bash
ctest --test-dir build -C Release
```

## Troubleshooting
*   **Driver Loading:** Ensure you have Test Mode enabled or are using a valid signing certificate.
*   **PDB Issues:** Ensure Visual Studio is configured to download symbols for system DLLs if you are debugging the overlay.
