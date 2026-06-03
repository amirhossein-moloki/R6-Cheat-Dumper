#include "driver.hpp"
#include "adder.h"
#include "finder.h"

uint64_t handle;
uint64_t base;

uint8_t* memory;

int main()
{
    printf("choose mode: 1 for kernel, 2 for user mode: ");
    int mode;
    std::cin >> mode;

    if (mode == 2) {
        driver::set_user_mode(true);
        printf("[+] user mode selected\n");
    } else {
        driver::set_user_mode(false);
        printf("[+] kernel mode selected\n");
    }

    if (!driver::initialize()) {
        std::cout << "[!] driver/interface init failed, falling back to user mode...\n";
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
        std::cout << "[-] failed to find rainbow six process!" << std::endl;
        std::cin.get();
        return 1;
    }

    handle = driver::open_process(pid);

    if (handle == 0) {
        std::cout << "[-] failed to open rainbow six process!" << std::endl;

        std::cin.get();
        return 1;
    }

    base = driver::get_module_base((uint64_t)pid, L"RainbowSix.exe");

    if (base == 0) {
        std::cout << "[-] failed to find module base!" << std::endl;

        std::cin.get();
        return 1;
    }

    memory = new uint8_t[seg_end(segment::data)];

    if (!driver::read_memory(handle, base, memory, (uint32_t)seg_end(segment::data))) {
        std::cout << "error reading memory\n";
        std::cin.get();
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