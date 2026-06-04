# Security Audit Report - memdrv.sys

## 1. Executive Summary
A comprehensive security audit was performed on the `memdrv` kernel driver. Several critical vulnerabilities and stability issues were identified, primarily related to user-mode buffer validation, safe process attachment, and race conditions during system information queries.

## 2. Critical Issues (Must Fix)

### 2.1 Insufficient Probing of Nested Pointers
- **Location:** `main.cpp`, `IOCTL_MEMORY_READ` / `IOCTL_MEMORY_WRITE`
- **Description:** The driver uses `METHOD_NEITHER`. While the primary request structure is probed, the `Buffer` pointer inside `MEMORY_REQUEST` is passed directly to memory routines without validation. A malicious user could provide a kernel-mode address, leading to arbitrary kernel memory read/write.
- **Fix:** Use `ProbeForRead`/`ProbeForWrite` on `request->Buffer` before use.

### 2.2 Unsafe PEB/Ldr Traversal
- **Location:** `util/memory.cpp` -> `GetProcessModuleBase`
- **Description:** The driver attaches to the target process and traverses its PEB and Ldr lists. These are user-mode structures and can be modified by the process at any time or become invalid. Accessing them without SEH (`__try/__except`) leads to a BSOD if the memory is paged out or tampered with.
- **Fix:** Wrap all PEB and Ldr access in `__try/__except` blocks.

### 2.3 Race Condition in Process Enumeration
- **Location:** `util/process.cpp` -> `GetProcessIdByName`
- **Description:** The code calls `ZwQuerySystemInformation` once to get the size and a second time to get the data. The process list can grow between these calls, causing the second call to fail with `STATUS_INFO_LENGTH_MISMATCH`.
- **Fix:** Implement a loop that retries the allocation and query until the buffer is large enough.

## 3. High Severity Issues (Should Fix)

### 3.1 Device Object Security
- **Location:** `main.cpp` -> `DriverEntry`
- **Description:** `IoCreateDevice` is called without an SDDL string. On some Windows versions, this might result in permissive access controls, allowing non-privileged users to interact with the driver.
- **Fix:** Use `IoCreateDeviceSecure` with an appropriate SDDL (e.g., `SDDL_DEVOBJ_SYS_ALL_ADM_ALL`).

### 3.2 Lack of Input Validation (String Lengths)
- **Location:** `main.cpp`, `defs.hpp`
- **Description:** IOCTLs like `IOCTL_MODULE_BASE` and `IOCTL_PROCESS_ID` use fixed-size buffers for strings. The driver does not guarantee that these strings are null-terminated before passing them to string comparison functions.
- **Fix:** Explicitly null-terminate the last character of the input buffers or use safe string functions like `RtlUnicodeStringInit`.

## 4. Medium & Low Severity Issues

### 4.1 Deprecated Pool Allocation (Minor)
- **Location:** `util/process.cpp`
- **Description:** While `ExAllocatePool2` is used (which is good), some parts of the code might still rely on older patterns.
- **Status:** Already partially addressed by the use of `ExAllocatePool2`.

### 4.2 Error Handling Consistency
- **Location:** Throughout `routines.cpp`
- **Description:** Some failures return `STATUS_NOT_FOUND` or `STATUS_NOT_IMPLEMENTED`. These should be mapped to more standard NTSTATUS codes where possible.

---

## 5. Exploitation Risk Assessment

| Attack Vector | Current Risk | Fixed Risk | Mitigation |
|---------------|--------------|------------|------------|
| Buffer overflow | Low | Negligible | Safe string handling and size validation |
| Arbitrary process read | High | Low | PID validation and proper pointer probing |
| Privilege escalation | High | Low | Secure Device Object and pointer validation |
| Denial of service | High | Low | SEH around all user-mode memory accesses |

---

## 6. BSOD Prevention Checklist

- [x] All user-mode accesses wrapped in try/except (Implemented in `main.cpp` and `util/memory.cpp`)
- [x] All process attachments balanced (`KeStackAttachProcess` / `KeUnstackDetachProcess` in `util/memory.cpp`)
- [x] No uninitialized variables (Verified in all updated files)
- [x] All allocated memory freed (Retry loop in `util/process.cpp` ensures pool memory is freed)
- [x] IRP completion guaranteed (Ensured in `main.cpp` IOCTL handler)
- [x] No hardcoded addresses (Dynamic resolution used for `MmCopyVirtualMemory`)
- [x] Pool tags unique and meaningful (`'PROC'` used for process enumeration)

## 7. Exploitation Risk Assessment - Final

| Attack Vector | Initial Risk | Final Risk | Mitigation implemented |
|---------------|--------------|------------|-------------------------|
| Buffer overflow | Low | Negligible | Safe string handling and size validation |
| Arbitrary process read | High | Low | PID validation and proper nested pointer probing |
| Privilege escalation | High | Low | Secure Device Object (SDDL) and pointer validation |
| Denial of service | High | Low | SEH around all user-mode memory accesses |
