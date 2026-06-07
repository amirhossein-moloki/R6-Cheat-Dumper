@echo off
setlocal enabledelayedexpansion

:: ============================================================================
:: Technical Suite CMake Build Helper
:: ============================================================================

echo [*] Initializing Technical Suite Build System...

:: 1. Detection & Generator Selection
echo [*] Detecting Visual Studio environment...

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
set "VS_VERSION="
set "GENERATOR="

if exist "!VSWHERE!" (
    for /f "usebackq tokens=1-3 delims=." %%i in (`"!VSWHERE!" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationVersion`) do (
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

if not exist build (
    mkdir build
)

cd build

if "!GENERATOR!"=="" (
    echo [!] Preferred Visual Studio version not detected via vswhere.
    echo [*] Attempting manual discovery...

    :: Try probing for common versions
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
    echo [!] No specific Visual Studio generator could be confirmed.
    echo [*] Falling back to CMake default...
    cmake .. -A x64
) else (
    echo [+] Selected: !GENERATOR!
    cmake .. -G "!GENERATOR!" -A x64
)

if !errorlevel! neq 0 (
    echo.
    echo [ERROR] CMake configuration failed.
    echo [TIP] Please ensure Visual Studio is installed with "Desktop development with C++".
    echo [TIP] If you have multiple versions, try running from a "Developer Command Prompt".
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
