# Forensic Analysis & Build Fixes Report

## 1. Analysis Results

### 1.1 memdrv (Kernel Driver)
- **Status**: Identified LNK4266 and LNK1218 errors.
- **Root Cause**: Missing specific kernel-mode compiler/linker flags and WDK library paths in the original CMake configuration. The linker was attempting to use standard user-mode defaults.
- **Fix**:
    - Explicitly set `/kernel`, `/GS-`, and `/Qspectre-`.
    - Configured linker for `/DRIVER`, `/ENTRY:DriverEntry`, and `/SUBSYSTEM:NATIVE`.
    - Linked `ntoskrnl.lib`, `hal.lib`, and `wdmsec.lib` from the WDK 10.0.28000.0 library path.

### 1.2 r6dumper (Offset Dumper)
- **Status**: Basic console application.
- **Root Cause**: Lack of proper SDK pathing and potential Spectre mitigation issues.
- **Fix**:
    - Synchronized SDK versions to 10.0.28000.0.
    - Disabled Spectre mitigation (`/Qspectre-`).
    - Added post-build step to copy `offsets.db`.

### 1.3 r6external (External Cheat)
- **Status**: Windows application with DirectX 11 overlay.
- **Root Cause**: Unresolved symbols and incorrect subsystem configuration.
- **Fix**:
    - Set subsystem to `/SUBSYSTEM:WINDOWS`.
    - Set entry point to `mainCRTStartup` to allow `int main()` usage in a Windows subsystem.
    - Linked `d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`, `winmm.lib`, `user32.lib`, and `gdi32.lib`.

## 2. Environment Compatibility
- **Visual Studio**: 2026 Enterprise (v145 toolset)
- **Windows SDK/WDK**: 10.0.28000.0
- **CMake**: 4.3.3

## 3. How to Verify
1. Run `.\build.bat` from the root directory.
2. Check the `build/Release` directory for:
    - `memdrv.sys`
    - `r6dumper.exe`
    - `rainbowsix-external.exe`
    - `offsets.db`

## 4. Common Pitfalls Avoided
- **Spectre Mitigations**: Disabled globally to prevent MSB8040 errors.
- **Control Flow Guard**: Disabled for the kernel driver to prevent compatibility issues.
- **Manifest Generation**: Disabled for the kernel driver as it's not supported in native subsystem.
