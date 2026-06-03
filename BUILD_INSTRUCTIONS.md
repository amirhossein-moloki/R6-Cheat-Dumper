# Build and Execution Instructions

This document provides technical instructions for building and running the Rainbow Six Siege cheat suite.

## Components
1. **memdrv**: Kernel-mode driver for memory access.
2. **r6dumper**: Offset dumper tool using signature scanning.
3. **r6external**: External cheat with ImGui overlay.

## Prerequisites
- **Visual Studio 2019/2022** with "Desktop development with C++".
- **Windows SDK** (Latest Version).
- **Windows Driver Kit (WDK)** (Required for `memdrv`).
- **NuGet Package Manager** (For dependency restoration).

## Building

### Kernel Driver (memdrv)
- Open `memdrv/memdrv.sln`.
- Set configuration to **Release | x64**.
- Build the project.
- *Alternative*: Build via command line:
  ```bash
  msbuild memdrv.sln /p:Configuration=Release /p:Platform=x64 /p:SpectreMitigation=false /t:Rebuild /v:minimal
  ```
- *Note*: Requires WDK. The driver creates a device named `\Device\MemDrv`. Must be loaded using a driver mapper or by enabling Test Mode.

### Offset Dumper (r6dumper)
- Open `r6dumper/r6od.sln`.
- Set configuration to **Release | x64**.
- Ensure `offsets.db` is present in the output directory.
- Build the project.

### External Cheat (r6external)
- Open `r6external/rainbowsix-external.sln`.
- Set configuration to **Release | x64**.
- Restore NuGet packages before building.
- This project builds as a **DLL**. Use a DLL injector or change project settings to **Application (.exe)** if preferred.
- *Note*: Uses a local version of ImGui and MinHook (located in `overlay/`).
- Build the project.

## Execution Sequence
1. Load `memdrv.sys`.
2. Launch Rainbow Six Siege.
3. (Optional) Run `r6dumper.exe` to update offsets.
4. Inject/Run `r6external`.

## Controls
- **INSERT**: Toggle Menu.
- **DELETE**: Exit Cheat.
