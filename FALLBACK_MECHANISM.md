# Technical Analysis: Dual-Mode Memory Access and Fallback Mechanism

## Overview
This technical suite has been updated to include a robust, dual-mode memory access interface that prioritizes kernel-level access via the `memdrv` driver while providing a seamless User-mode fallback using standard Win32 APIs.

## 1. Primary Mode: Kernel Driver
The primary communication method remains the `memdrv` kernel driver. It utilizes `DeviceIoControl` (or a custom syscall-like mechanism in the external project) to perform memory operations.
- **Benefits:** Bypasses many security callbacks and handle stripping mechanisms.
- **Mechanism:** Communication via `\\.\Global\MemDrv`.

## 2. Fallback Mode: User-Mode (ReadProcessMemory)
When the driver is not loaded or the device cannot be opened, the system automatically transitions to User-mode.

### Limited Rights Handle
The fallback utilizes a specific strategy for obtaining a process handle. Instead of requesting `PROCESS_ALL_ACCESS` (which is frequently blocked or stripped by security modules), it requests only the minimum necessary rights:
```cpp
HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
```
- **PROCESS_QUERY_INFORMATION:** Required for `GetProcessId` and some module enumeration tasks.
- **PROCESS_VM_READ:** Required for `ReadProcessMemory`.

### Why it works
Many anti-cheat systems employ "StripHandle" callbacks (`ObRegisterCallbacks`) to remove powerful rights like `PROCESS_ALL_ACCESS`, `PROCESS_VM_WRITE`, or `PROCESS_TERMINATE`. However, they sometimes leave basic query and read rights intact to maintain system compatibility or for diagnostic purposes. This "loophole" allows the technical suite to function even without a driver, albeit with limitations.

## 3. Limitations
- **Read Restrictions:** Security modules may still protect specific memory regions (like the `.text` section of the main executable) even if a read handle is granted. In such cases, `ReadProcessMemory` will return `FALSE`.
- **Write Operations:** `PROCESS_VM_WRITE` is almost always stripped by BattlEye. Consequently, features requiring memory modification (like "No Recoil") will fail in User-mode fallback.
- **Detection Risk:** Using User-mode handles is significantly more "visible" to security scanners than driver-based access.

## 4. Diagnostics and Stability
- **Auto-Switching:** The `driver::initialize()` function handles the detection and switching logic automatically.
- **Error Reporting:** All critical failures now print the exact Windows error code (`GetLastError()`) and a description.
- **Console Persistence:** To prevent the application from closing before the user can read the error message, `system("pause")` or similar blocks are used on all fatal error paths.

## 5. Build Instructions
The project remains compatible with standard MSVC compilers.
- **Toolset:** Visual Studio 2022/2025 (v143 or later).
- **Architecture:** x64.
- **Command Line:** `msbuild project_name.sln /p:Configuration=Release /p:Platform=x64`
