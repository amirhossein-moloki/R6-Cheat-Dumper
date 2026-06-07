@echo off
setlocal enabledelayedexpansion

:: ============================================================================
:: Technical Suite CMake Build Helper
:: ============================================================================

echo [*] Initializing Technical Suite Build System...

if not exist build (
    mkdir build
)

cd build

:: 1. Intelligent Generator Selection
echo [*] Selecting CMake Generator...

set "GENERATOR="

:: Try VS 2022
cmake .. -G "Visual Studio 17 2022" -A x64 -T v143 >nul 2>&1
if !errorlevel! equ 0 (
    set "GENERATOR=Visual Studio 17 2022"
) else (
    :: Try VS 2019
    cmake .. -G "Visual Studio 16 2019" -A x64 -T v142 >nul 2>&1
    if !errorlevel! equ 0 (
        set "GENERATOR=Visual Studio 16 2019"
    )
)

if "!GENERATOR!"=="" (
    echo [!] Preferred Visual Studio generators not found.
    echo [*] Falling back to default generator...
    cmake .. -A x64
) else (
    echo [+] Selected: !GENERATOR!
    :: Re-run with output for final confirmation
    cmake .. -G "!GENERATOR!" -A x64
)

if !errorlevel! neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b !errorlevel!
)

:: 2. Build Project
echo [*] Building Technical Suite (Release)...
cmake --build . --config Release

if !errorlevel! neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b !errorlevel!
)

echo.
echo [+] Technical Suite Build Complete!
pause
exit /b 0
