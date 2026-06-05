#include <iostream>
#include <thread>
#include <Windows.h>

#include "util/xorstr.hpp"

#include "driver/driver.hpp"
#include "features/cheat.hpp"
#include "features/visuals/visuals.hpp"
#include "game/game_util.h"
#include "globals.hpp"
#include "overlay/input/input.hpp"
#include "overlay/overlay.hpp"

#include "util/util.hpp"
#include "util/memory.hpp"

#include "core/cheat_context.hpp"
#include "core/memory_service.hpp"

void RunCheat() {
	std::cout << "[*] Initializing technical suite (Phase 2 Refactored)..." << std::endl;

    auto context = std::make_shared<core::CheatContext>();
    auto memory_service = std::make_shared<core::MemoryService>();

    context->set_memory_service(memory_service);

	if (!memory_service->initialize()) {
		std::cout << "[-] Critical failure during memory service initialization." << std::endl;
		system("pause");
		exit(1);
	}

	if (!memory_service->is_kernel_mode()) {
		std::cout << "[*] User-mode interface initialized. Access level will be determined upon attachment." << std::endl;
	}
	else {
		std::cout << "[+] Kernel-mode driver interface initialized (Full Access)." << std::endl;
	}

	overlay::enable();

	std::cout << "[*] Looking for RainbowSix.exe..." << std::endl;
    context->set_state(core::CheatState::WaitingForProcess);
	while (!util::is_game_open("Rainbow Six", "R6Game", "RainbowSix.exe")) {
		if (overlay::input::key_pressed(VK_DELETE))
			exit(0);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	uint32_t game_pid = util::get_pid_from_file("RainbowSix.exe");
    context->set_game_pid(game_pid);
    globals::game_pid = game_pid; // Keep legacy globals in sync during transition

	if (game_pid == 0) {
		std::cout << "[-] Invalid process id." << std::endl;
		system("pause");
		exit(1);
	}

	if (!memory_service->is_kernel_mode()) {
		if (memory_service->has_write_access()) {
			std::cout << "[+] Attached with FULL ACCESS (Read/Write)." << std::endl;
		}
		else {
			std::cout << "[!] Attached with LIMITED ACCESS (Read-Only)." << std::endl;
			std::cout << "[!] Write-based features (No Recoil, Glow, etc.) will be disabled." << std::endl;
		}
	}
	
	std::cout << "[+] Found PID: " << game_pid << std::endl;

	if (!memory_service->attach(game_pid)) {
		std::cout << "[-] Failed to attach to process memory." << std::endl;
		system("pause");
		exit(1);
	}

    uintptr_t module_base = memory_service->get_module_base(L"RainbowSix.exe");
	if (module_base == 0) {
		module_base = memory_service->get_module_base(L"RainbowSix_Vulkan.exe");
	}
    context->set_module_base(module_base);
    globals::module_base = module_base; // Keep legacy globals in sync during transition

	if (module_base == 0) {
		std::cout << "[-] Failed to get module base address." << std::endl;
		system("pause");
		exit(1);
	}

	std::cout << "[+] Module base: 0x" << std::hex << module_base << std::dec << std::endl;

	Beep(500, 500);

	std::cout << "[*] Updating game addresses..." << std::endl;
    // Note: game::update_addresses still uses globals for now, will refactor in later steps
	while (!game::update_addresses()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		std::cout << "[*] Still waiting for game addresses to update..." << std::endl;
	}
	std::cout << "[+] Game addresses updated." << std::endl;
	
    context->set_state(core::CheatState::Attached);

	std::cout << "[*] Cheat loop started. Press DELETE to exit." << std::endl;
	while (overlay::input::key_pressed(VK_DELETE) == 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		
        // Frame-based cache invalidation for performance and consistency
        memory_service->clear_cache();
        memory_service->enable_caching(true);

		if (!game::in_match() || game::get_profile() == 0) {
            context->set_state(core::CheatState::Attached);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			continue;
		}

        context->set_state(core::CheatState::InMatch);
		cheat::run();
	}

	cheat::restore();

	overlay::disable();

	Beep(500, 500);
	
	exit(0);
}

int main() {
    RunCheat();
    return 0;
}
