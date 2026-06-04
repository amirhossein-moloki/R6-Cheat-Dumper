# R6 Dumper Technical Analysis

## 1. Architecture Overview

The `r6dumper` is a sophisticated reverse engineering utility designed to automate the discovery of memory offsets within the game process. It operates as part of a three-component technical suite, collaborating with a kernel driver (`memdrv`) to achieve stealthy and unrestricted memory access.

### Communication Flow
The dumper utilizes a dual-mode communication architecture abstracted within the `driver` namespace:

1.  **Kernel Mode (Ring 0):** This is the primary mode for bypassing security. The dumper communicates with the `\Device\Global\MemDrv` device via `DeviceIoControl`. This allows the dumper to request memory operations that the kernel driver executes using `MmCopyVirtualMemory`, bypassing User-mode hooks and security callbacks.
2.  **User Mode (Ring 3):** A fallback mode that uses standard Win32 APIs (`OpenProcess`, `ReadProcessMemory`). This is useful for development or testing on systems where the driver is not loaded.

### Stealth Mechanism: Offline Scanning
Unlike many dumpers that perform hundreds of small reads while scanning, `r6dumper` follows a high-performance, low-detection flow:
- It identifies the `.text`, `.rdata`, and `.data` segments of the game.
- It performs a few large memory copies to read these entire segments into its own local process memory (`uint8_t* memory`).
- All subsequent pattern matching, XRef analysis, and signature scanning are performed on this local buffer. This minimizes the number of "cross-process" operations, which are often monitored by anti-cheat solutions.

---

## 2. Complete Dumping Flow

### Step 1: Process Discovery
The dumper first attempts to find the game process. It prioritizes `CreateToolhelp32Snapshot` to look for `RainbowSix.exe` or `RainbowSix_Vulkan.exe`. If the process is not found by name, it falls back to `FindWindowA` as a secondary detection method.

### Step 2: Module Base Acquisition
Once the PID is obtained, the dumper retrieves the base address of the main module. In Kernel-mode, the driver enumerates the process's `Ldr` list to find the module. In User-mode, it uses `Module32First/Next`.

### Step 3: Segment Buffering
The dumper maps the game's memory structure. Based on predefined (or scanned) segment boundaries, it allocates a local buffer and copies the game's executable code and data sections.

### Step 4: The "Adder" vs. "Finder" Loop
The dumper provides two main workflows:
-   **Adder (Signature Creation):** The user provides a known offset (found via manual reversing in IDA/Reclass). The dumper analyzes the area around that offset to create a unique "fingerprint" (often using String XRefs or Hex Dumps).
-   **Finder (Signature Relocation):** The dumper reads the `offsets.db` file, parses the fingerprints, and searches the current game memory buffer to find where those offsets have moved after a game update.

### Step 5: Persistence
Found offsets are either printed to the console or saved/updated in the `offsets.db` database for use by the `r6external` cheat component.

---

## 3. Function Deep Dive

### `shared::sig_scan`
- **What:** A multi-threaded pattern matching engine.
- **How:** It divides the target memory segment into chunks and assigns each to a thread. Each thread performs a byte-by-byte comparison against a target signature.
- **Why:** Scanning 100MB+ of memory is slow. Multi-threading (defaulting to 6 threads) significantly reduces the time required to relocate offsets.

### `shared::find_xrefs`
- **What:** Finds "Cross-References" to a specific memory address.
- **How:** It scans the `.text` (code) segment for instructions that reference the target address. It supports both absolute (64-bit) and relative (RIP-relative) addressing.
- **Why:** Variables like the "GameManager" or "EntityList" are often referenced by specific functions. If you know a unique string or variable address, you can find the code that uses it.

### `shared::extract_relative_offset`
- **What:** Converts a RIP-relative instruction operand into an absolute virtual address.
- **How:** In x64, many instructions use a 32-bit displacement relative to the next instruction's address. This function reads that 4-byte displacement and adds it to the current instruction pointer (`RIP + displacement + 7`).
- **Why:** This is essential for resolving addresses of global variables and functions in 64-bit executables.

---

## 4. Anti-Cheat Evasion

### Bypassing BattlEye
BattlEye (BE) heavily monitors User-mode activity. The `memdrv` component allows `r6dumper` to:
1.  **Bypass Handle Stripping:** BE often uses `ObRegisterCallbacks` to strip `PROCESS_ALL_ACCESS` rights from handles to the game. The kernel driver doesn't need a standard Win32 handle; it uses the PID and direct kernel pointers.
2.  **Avoid Memory Hooks:** BE hooks `ReadProcessMemory` in many processes. By using `DeviceIoControl` to a custom driver, the dumper's memory requests never pass through the hooked Win32 functions.

### Detection Risks
While kernel access is powerful, it is not invisible:
-   **System Thread/Module Enumeration:** Anti-cheats can scan for unsigned or "manual mapped" drivers.
-   **IOCTL Monitoring:** Advanced anti-cheats might monitor communication to unknown device objects.
-   **Communication Pattern:** Even if the *method* of reading is safe, *what* is being read (like the entire .text section) can be flagged if not done carefully.

---

## 5. Code Analysis

### Pattern Scanning Algorithm
The dumper uses a multi-threaded scanner that relies on a comparison function with wildcard support. Note that in this implementation, the function returns `false` when a match is found:

```cpp
static bool cmp_sig(uint8_t sig[], uint64_t address, size_t len, const char* skips = "") {
    if (skips == "") {
        if (!memcmp(sig, memory + address, len))
            return false; // Match found
    }
    else {
        for (size_t i = 0; i < len; i++) {
            // '0' in the mask means the byte MUST match.
            // Any other character (like '?') is treated as a wildcard.
            if (skips[i] == '0' && memory[address + i] != sig[i])
                break;

            if (i == len - 1)
                return false; // All required bytes matched
        }
    }
    return true; // No match
}
```

### Signature Format in `offsets.db`
The dumper uses a custom delimited format to store identification logic. A typical entry looks like:
`Name|/|/|/|||||...data...////`

The database uses several unique separators defined in `adder.h`:
- `NAME_SEPERATOR` (`|/|/|/`): Separates the offset name from the data.
- `STRING_SEPERATOR` (`\|\|\|\|`): Marks the start of a string-based signature.
- `SEPERATOR` (`||||`): Separates the string content from its offset.
- `OPCODE_SEPERATOR` (`/|/|/|`): Separates the string offset from the instruction opcodes (REX, Instruction, Register).
- `ENTRY_SEPERATOR` (`////`): Marks the end of a single offset entry.

### Pattern Signature Format and Wildcards
The dumper's `cmp_sig` function handles wildcards using a "skips" mask. In this specific implementation:
- A `skips` character of `'0'` means the byte **must match** exactly.
- Any other character in the mask acts as a **wildcard** (equivalent to `??` in standard IDA patterns).

For example, a signature like `48 8B 0D ?? ?? ?? ?? 48 85 C9` would be represented internally as a byte array of the values, and a mask where the `??` positions do not have a `'0'` in the corresponding mask index.

### Memory Reading through Driver
```cpp
bool KernelInterface::ReadMemory(HANDLE pid, UINT64 address, void* buffer, UINT64 size) {
    MEMORY_REQUEST request;
    request.ProcessId = pid;
    request.Address = address;
    request.Buffer = buffer;
    request.Size = size;

    // Direct IOCTL to the kernel driver
    return DeviceIoControl(m_hDevice, IOCTL_MEMORY_READ, &request, sizeof(request), &request, sizeof(request), NULL, NULL);
}
```

---

## 6. Troubleshooting Guide

### Why offsets might not be found
1.  **Game Update:** Major updates often change the code structure, breaking signatures or moving strings to different functions.
2.  **Segment Shift:** If the `.text` or `.data` segment boundaries change, the hardcoded `seg_start_arr` and `seg_end_arr` in `shared.h` must be updated.
3.  **Instruction Changes:** A simple compiler optimization change (e.g., changing a `mov` to a `lea`) will break an XRef signature if it's looking for specific opcodes.

### Updating Signatures
When a signature breaks:
1.  Open the latest game executable in **IDA Pro**.
2.  Locate the desired variable manually (using strings or known constants).
3.  Run `r6dumper` in **Adder** mode.
4.  Input the new hex offset and a name.
5.  The dumper will generate a new, unique signature and save it to `offsets.db`.

### Debugging
-   **Driver Load Failure:** Ensure the driver is signed or the system is in Test Mode / DSE is disabled.
-   **Wrong Base Address:** Verify the module name (Standard vs Vulkan) matches what is being searched for in `main.cpp`.
