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

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VS_VERSION="
set "VS_PATH="
set "GENERATOR="

if exist "!VSWHERE!" (
    for /f "usebackq tokens=*" %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
    for /f "usebackq tokens=1 delims=." %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion`) do (
        set "VS_VERSION=%%i"
    )
)

if "!VS_VERSION!"=="18" (
    set "GENERATOR=Visual Studio 18 2026"
) else if "!VS_VERSION!"=="17" (
    set "GENERATOR=Visual Studio 17 2022"
) else if "!VS_VERSION!"=="16" (
    set "GENERATOR=Visual Studio 16 2019"
) else if "!VS_VERSION!"=="15" (
    set "GENERATOR=Visual Studio 15 2017"
)

if "!VS_PATH!"=="" (
    echo [ERROR] Could not locate Visual Studio installation via vswhere.
    echo Please ensure Visual Studio is installed with C++ development tools.
    pause
    exit /b 1
)

echo [+] Found VS at: !VS_PATH!
if not "!GENERATOR!"=="" (
    echo [+] Target Generator: !GENERATOR!
)

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

if "!GENERATOR!"=="" (
    echo [!] No specific generator mapped for detected VS version !VS_VERSION!.
    echo [*] Attempting manual discovery...

    cmake .. -G "Visual Studio 18 2026" -A x64 >nul 2>&1
    if !errorlevel! equ 0 (
        set "GENERATOR=Visual Studio 18 2026"
    ) else (
        cmake .. -G "Visual Studio 17 2022" -A x64 >nul 2>&1
        if !errorlevel! equ 0 (
            set "GENERATOR=Visual Studio 17 2022"
        ) else (
            cmake .. -G "Visual Studio 16 2019" -A x64 >nul 2>&1
            if !errorlevel! equ 0 (
                set "GENERATOR=Visual Studio 16 2019"
            )
        )
    )
)

if "!GENERATOR!"=="" (
    echo [*] Falling back to default generator...
    cmake .. -A x64
) else (
    echo [+] Using Generator: !GENERATOR!
    cmake .. -G "!GENERATOR!" -A x64
)

if !errorlevel! neq 0 (
    echo.
    echo [ERROR] CMake configuration failed.
    echo [TIP] Please ensure Windows SDK and WDK are installed correctly.
    pause
    exit /b !errorlevel!
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
