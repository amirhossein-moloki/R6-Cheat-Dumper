@echo off
echo [*] Initializing Technical Suite Build System (CMake)...
echo [*] Target Toolset: v145
echo [*] Target Platform: x64

if not exist "build" (
    mkdir build
)

cd build

:: Note: "Visual Studio 18 2026" is a future/hypothetical version.
:: We will use the latest available generator if it fails, but following instructions:
cmake .. -G "Visual Studio 18 2026" -A x64 -T v145

if %errorlevel% neq 0 (
    echo [!] "Visual Studio 18 2026" generator not found. Falling back to default Visual Studio generator with v145 toolset...
    cmake .. -A x64 -T v145
)

if %errorlevel% neq 0 (
    echo [-] CMake configuration failed.
    pause
    exit /b %errorlevel%
)

echo [*] Building Technical Suite (Release)...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo [-] Build failed.
    pause
    exit /b %errorlevel%
)

echo [+] Technical Suite Build Complete!
echo [+] Binaries are located in the build/bin/ directory (or component subdirectories).
pause
