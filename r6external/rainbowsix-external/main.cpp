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

void RunCheat() {
	std::cout << "[*] Initializing technical suite..." << std::endl;

	if (!driver::initialize()) {
		std::cout << "[-] Critical failure during driver interface initialization." << std::endl;
		system("pause");
		exit(1);
	}

	if (driver::g_user_mode) {
		std::cout << "[*] User-mode interface initialized. Access level will be determined upon attachment." << std::endl;
	}
	else {
		std::cout << "[+] Kernel-mode driver interface initialized (Full Access)." << std::endl;
	}

	overlay::enable();

	std::cout << "[*] Looking for RainbowSix.exe..." << std::endl;
	while (!util::is_game_open("Rainbow Six", "R6Game", "RainbowSix.exe")) {
		if (overlay::input::key_pressed(VK_DELETE))
			exit(0);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	globals::game_pid = util::get_pid_from_file("RainbowSix.exe");

	if (globals::game_pid == 0) {
		std::cout << "[-] Invalid process id." << std::endl;
		system("pause");
		exit(1);
	}

	if (driver::g_user_mode) {
		if (driver::g_has_write_access) {
			std::cout << "[+] Attached with FULL ACCESS (Read/Write)." << std::endl;
		}
		else {
			std::cout << "[!] Attached with LIMITED ACCESS (Read-Only)." << std::endl;
			std::cout << "[!] Write-based features (No Recoil, Glow, etc.) will be disabled." << std::endl;
		}
	}
	
	std::cout << "[+] Found PID: " << globals::game_pid << std::endl;

	if (!globals::memory.attach(globals::game_pid)) {
		std::cout << "[-] Failed to attach to process memory." << std::endl;
		system("pause");
		exit(1);
	}

	globals::module_base = driver::get_module_base(globals::game_pid, L"RainbowSix.exe");
	if (globals::module_base == 0) {
		globals::module_base = driver::get_module_base(globals::game_pid, L"RainbowSix_Vulkan.exe");
	}

	if (globals::module_base == 0) {
		std::cout << "[-] Failed to get module base address." << std::endl;
		system("pause");
		exit(1);
	}

	std::cout << "[+] Module base: 0x" << std::hex << globals::module_base << std::dec << std::endl;

	Beep(500, 500);

	std::cout << "[*] Updating game addresses..." << std::endl;
	while (!game::update_addresses()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
		std::cout << "[*] Still waiting for game addresses to update..." << std::endl;
	}
	std::cout << "[+] Game addresses updated." << std::endl;
	
	std::cout << "[*] Cheat loop started. Press DELETE to exit." << std::endl;
	while (overlay::input::key_pressed(VK_DELETE) == 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		
		//visuals::drone(true);

		if (!game::in_match() || game::get_profile() == 0) {
			std::this_thread::sleep_for(std::chrono::milliseconds(1)); // why not save some cpu?
			continue;
		}

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
