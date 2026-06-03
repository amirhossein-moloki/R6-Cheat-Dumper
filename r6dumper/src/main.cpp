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

    if (!driver::initialize())
        std::cout << "driver/interface init failed\n";

    DWORD pid;
    GetWindowThreadProcessId(FindWindowA(nullptr, "Rainbow Six"), &pid);

    handle = driver::open_process(pid);

    if (handle == 0) {
        std::cout << "[-] failed to find rainbow six!" << std::endl;

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