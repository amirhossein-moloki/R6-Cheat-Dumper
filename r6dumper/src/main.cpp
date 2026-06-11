#include "driver.hpp"
#include "adder.h"
#include "finder.h"
#include "logger.hpp"

uint64_t handle;
uint64_t base;

uint8_t* memory;

int main()
{
    logger::initialize();
    LOG_INFO("Initializing R6 Dumper...");

    printf("choose mode: 1 for kernel, 2 for user mode: ");
    int mode;
    std::cin >> mode;

    if (mode == 2) {
        driver::set_user_mode(true);
        LOG_INFO("User mode selected");
    } else {
        driver::set_user_mode(false);
        LOG_INFO("Kernel mode selected");
    }

    if (!driver::initialize()) {
        LOG_WARN("Driver/interface init failed, falling back to user mode...");
        driver::set_user_mode(true);
    }

    DWORD pid = 0;

    // Professional approach: Scan process list first for exact executable matches
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W processEntry;
        processEntry.dwSize = sizeof(processEntry);
        if (Process32FirstW(snapshot, &processEntry)) {
            do {
                // Check for standard and Vulkan versions of the game
                if (wcscmp(processEntry.szExeFile, L"RainbowSix.exe") == 0 ||
                    wcscmp(processEntry.szExeFile, L"RainbowSix_Vulkan.exe") == 0) {
                    pid = processEntry.th32ProcessID;
                    break; // Found the actual game process
                }
                // Also check for the BE launcher as a fallback/hint
                if (wcscmp(processEntry.szExeFile, L"RainbowSix_BE.exe") == 0) {
                    pid = processEntry.th32ProcessID;
                }
            } while (Process32NextW(snapshot, &processEntry));
        }
        CloseHandle(snapshot);
    }

    // Fallback to window title if process snapshot didn't find the main executables
    if (pid == 0) {
        HWND window = FindWindowA(nullptr, "Rainbow Six");
        if (window) {
            GetWindowThreadProcessId(window, &pid);
        }
    }

    if (pid == 0) {
        LOG_ERROR("Failed to find Rainbow Six process! Listing active processes:");
        HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnap != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32W pe;
            pe.dwSize = sizeof(pe);
            if (Process32FirstW(hSnap, &pe)) {
                do {
                    LOG_INFO("  - {} (PID: {})", shared::ws2s(pe.szExeFile), pe.th32ProcessID);
                } while (Process32NextW(hSnap, &pe));
            }
            CloseHandle(hSnap);
        }
        system("pause");
        return 1;
    }

    LOG_INFO("Found PID: {}", pid);

    handle = driver::open_process(pid);

    if (handle == 0) {
        LOG_ERROR("Failed to open Rainbow Six process!");
        system("pause");
        return 1;
    }

    base = driver::get_module_base(handle, L"RainbowSix.exe");
    if (base == 0) {
        base = driver::get_module_base(handle, L"RainbowSix_Vulkan.exe");
    }

    if (base == 0) {
        LOG_ERROR("Failed to find module base!");
        system("pause");
        return 1;
    }

    LOG_INFO("Module base: 0x{:x}", base);

    // Discovery phase: Read PE headers first
    uint8_t header[0x1000];
    if (!driver::read_memory(handle, base, header, sizeof(header))) {
        LOG_ERROR("Failed to read PE headers!");
        system("pause");
        return 1;
    }

    if (!shared::discover_segments(header)) {
        LOG_ERROR("Failed to discover segments from PE headers!");
        system("pause");
        return 1;
    }

    uint64_t total_size = seg_end_arr[2]; // .data end
    LOG_INFO("Discovered segments. Total size to map: 0x{:x}", total_size);
    LOG_DEBUG("    .text:  0x{:x} - 0x{:x}", seg_start_arr[0], seg_end_arr[0]);
    LOG_DEBUG("    .rdata: 0x{:x} - 0x{:x}", seg_start_arr[1], seg_end_arr[1]);
    LOG_DEBUG("    .data:  0x{:x} - 0x{:x}", seg_start_arr[2], seg_end_arr[2]);

    memory = new uint8_t[total_size];

    if (!driver::read_memory(handle, base, memory, (uint32_t)total_size)) {
        LOG_ERROR("Error reading full process memory.");
        system("pause");
        return 1;
    }

    while (true) {
        printf("do you want to add an offset? (y/n)");
        char ans;
        std::cin >> ans;

        if (ans == 'y')
            adder::run();
        else {
            finder::run();
            break;
        }
    }

    return 1;
}