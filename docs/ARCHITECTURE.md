# System Architecture Documentation

## Overview
The Technical Suite is a modular Windows-based system designed for low-level memory introspection and technical analysis. It consists of three primary components: a kernel driver, a memory dumper, and an external application.

## Component Hierarchy

### 1. Kernel Driver (`memdrv`)
*   **Purpose:** Provides a secure and privileged interface for memory operations.
*   **Communication:** Uses IOCTLs (Buffered I/O) to communicate with user-mode components.
*   **Security:** Implements strict SDDLs and buffer validation to prevent unauthorized access.

### 2. External Application (`r6external`)
*   **Architecture:** Service-Oriented Architecture (SOA) with a central `CheatContext`.
*   **Core Services:**
    *   **MemoryService:** Manages read/write operations with a caching layer. Supports both Kernel-mode (IOCTL) and User-mode (Win32 API) fallbacks.
    *   **ConfigService:** Handles JSON-based configuration persistence.
    *   **Logger:** Provides structured logging via `spdlog`.
*   **UI:** Implements a DirectX 11 overlay with ImGui.

### 3. Offset Dumper (`r6dumper`)
*   **Purpose:** Automates the discovery of memory offsets via signature scanning.
*   **Features:** Multi-threaded scanning, dynamic PE header parsing, and an interactive offset database.

## Design Patterns

### Dependency Injection (DI)
The system uses DI to manage service lifecycles. The `CheatContext` acts as the container, holding shared pointers to various services. Features receive a reference to the `CheatContext` or specific services during initialization.

### Service-Oriented Architecture
Functionality is encapsulated into discrete services (e.g., `IMemoryService`) with clearly defined interfaces. This promotes testability and allows for easy implementation swaps (e.g., mocking memory for unit tests).

### RAII (Resource Acquisition Is Initialization)
Critical resources like process handles and kernel device handles are managed using RAII wrappers to ensure they are properly released even in the event of exceptions.

## Data Flow
1.  **Initialization:** `main.cpp` initializes the `Logger` and `CheatContext`.
2.  **Configuration:** `ConfigService` loads settings from `config.json`.
3.  **Attachment:** `MemoryService` attempts to connect to the kernel driver, falling back to Win32 APIs if necessary, and then attaches to the target process.
4.  **Main Loop:** The application enters a frame-based loop where services update their state, features execute their logic, and the overlay renders the results.
