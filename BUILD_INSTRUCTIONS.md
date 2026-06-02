# Build and Execution Instructions

This document provides technical instructions for building and running the Rainbow Six Siege cheat suite.

## Components
1. **memdrv**: Kernel-mode driver for memory access.
2. **r6dumper**: Offset dumper tool using signature scanning.
3. **r6external**: External cheat with ImGui overlay.

## Prerequisites
- **Visual Studio 2019/2022** with "Desktop development with C++".
- **Windows SDK**.
- **Windows Driver Kit (WDK)** (Required for `memdrv`).

## Building

### Kernel Driver (memdrv)
- Open `memdrv/memdrv.sln`.
- Set configuration to **Release | x64**.
- Build the project.
- *Note*: Requires WDK. The project is now configured to use the latest installed Windows SDK and toolset for better portability.

### Offset Dumper (r6dumper)
- Open `r6dumper/r6od.sln`.
- Set configuration to **Release | x64**.
- Ensure `offsets.db` is present in the output directory.
- Build the project.

### External Cheat (r6external)
- Open `r6external/rainbowsix-external.sln`.
- Visual Studio will automatically restore the **MinHook** NuGet package.
- Set configuration to **Release | x64**.
- This project builds as a **DLL**. Use a DLL injector or change project settings to **Application (.exe)** if preferred.
- Build the project.

## Execution Sequence
1. Load `memdrv.sys`.
2. Launch Rainbow Six Siege.
3. (Optional) Run `r6dumper.exe` to update offsets.
4. Inject/Run `r6external`.

## Controls
- **INSERT**: Toggle Menu.
- **DELETE**: Exit Cheat.
