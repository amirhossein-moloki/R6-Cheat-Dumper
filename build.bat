@echo off
setlocal enabledelayedexpansion

echo [*] Starting Technical Suite Build Process...

:: 1. Detect Visual Studio Environment
echo [*] Detecting Visual Studio environment...
set "VS_PATH=C:\Program Files\Microsoft Visual Studio\2026\Enterprise"
if not exist "!VS_PATH!" (
    echo [!] Visual Studio 2026 Enterprise not found at !VS_PATH!
    echo [*] Attempting to find via vswhere...
    for /f "usebackq tokens=*" %%i in (`"C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
        set "VS_PATH=%%i"
    )
)

if not exist "!VS_PATH!" (
    echo [ERROR] Could not locate Visual Studio installation.
    pause
    exit /b 1
)

echo [+] Found VS at: !VS_PATH!

:: 2. Create Build Directory
if exist build (
    echo [*] Cleaning previous build directory...
    rd /s /q build
)
mkdir build
cd build

:: 3. Run CMake Configuration
echo [*] Configuring project with CMake...
cmake .. -G "Visual Studio 18 2026" -A x64
if %errorlevel% neq 0 (
    echo [ERROR] CMake configuration failed.
    pause
    exit /b %errorlevel%
)

:: 4. Run CMake Build
echo [*] Building project (Release x64)...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo [ERROR] Build failed.
    pause
    exit /b %errorlevel%
)

echo.
echo [+] Build configuration complete!
echo [+] memdrv.sys built successfully
echo [+] r6dumper.exe built successfully
echo [+] rainbowsix-external.exe built successfully
echo [+] All components ready in build/Release folder.

pause
exit /b 0
