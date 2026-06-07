@echo off
setlocal enabledelayedexpansion

:: ============================================================================
:: Technical Suite Build Script
:: ============================================================================

set "CONFIG=Release"
set "CLEAN=false"

:parse_args
if "%~1"=="" goto end_parse
if /i "%~1"=="--debug" (
    set "CONFIG=Debug"
) else if /i "%~1"=="--release" (
    set "CONFIG=Release"
) else if /i "%~1"=="--clean" (
    set "CLEAN=true"
) else (
    echo [!] Unknown argument: %~1
    echo Usage: build.bat [--debug|--release] [--clean]
    exit /b 1
)
shift
goto parse_args
:end_parse

echo [*] Starting Technical Suite Build Process...
echo [*] Configuration: !CONFIG!

:: 1. Detect Visual Studio Environment
echo [*] Detecting Visual Studio environment...
set "VS_PATH="
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if exist "!VSWHERE!" (
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

if "!VS_PATH!"=="" (
    echo [ERROR] Could not locate Visual Studio installation.
    echo Please ensure Visual Studio is installed with C++ development tools.
    pause
    exit /b 1
)

echo [+] Found VS at: !VS_PATH!

:: 2. Clean/Create Build Directory
if "!CLEAN!"=="true" (
    if exist build (
        echo [*] Cleaning build directory...
        rd /s /q build
    )
)

if not exist build (
    mkdir build
)

cd build

:: 3. Run CMake Configuration
echo [*] Configuring project with CMake...
:: Try latest generators
set "GENERATOR="
cmake .. -G "Visual Studio 17 2022" -A x64 >nul 2>&1
if !errorlevel! equ 0 (
    set "GENERATOR=Visual Studio 17 2022"
) else (
    cmake .. -G "Visual Studio 16 2019" -A x64 >nul 2>&1
    if !errorlevel! equ 0 (
        set "GENERATOR=Visual Studio 16 2019"
    )
)

if "!GENERATOR!"=="" (
    echo [*] Falling back to default generator...
    cmake .. -A x64
    if !errorlevel! neq 0 (
        echo [ERROR] CMake configuration failed.
        pause
        exit /b !errorlevel!
    )
) else (
    echo [+] Using Generator: !GENERATOR!
    :: Configuration already done by the test command above, but we re-run to show output
    cmake .. -G "!GENERATOR!" -A x64
)

:: 4. Run CMake Build
echo [*] Building project (!CONFIG! x64)...
cmake --build . --config !CONFIG!
if !errorlevel! neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b !errorlevel!
)

echo.
echo [+] Build process completed successfully!
echo [+] Binaries are available in build/bin/ or respective component folders.

pause
exit /b 0
