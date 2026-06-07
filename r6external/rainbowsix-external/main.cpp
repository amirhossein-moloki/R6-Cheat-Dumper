#include <thread>
#include <Windows.h>

#include "util/xorstr.hpp"

#include "driver/driver.hpp"
#include "features/cheat.hpp"
#include "features/visuals/visuals.hpp"
#include "game/game_util.h"
#include "config.hpp"
#include "overlay/input/input.hpp"
#include "overlay/overlay.hpp"

#include "util/util.hpp"
#include "util/memory.hpp"

#include "core/cheat_context.hpp"
#include "core/memory_service.hpp"
#include "core/config_service.hpp"
#include "core/health_service.hpp"
#include "core/performance_metrics.hpp"
#include "core/logger.hpp"

void RunCheat() {
    core::Logger::initialize();
    LOG_INFO("Initializing Technical Suite v1.1.0 (Phase 6 Enterprise)...");

    auto context = std::make_shared<core::CheatContext>();
    auto memory_service = std::make_shared<core::MemoryService>();
    auto config_service = std::make_shared<core::ConfigService>();
    auto health_service = std::make_shared<core::HealthService>(context);

    context->set_memory_service(memory_service);
    context->set_config_service(config_service);
    context->set_health_service(health_service);

    if (!config_service->initialize()) {
        LOG_ERROR("Failed to initialize configuration service.");
        system("pause");
        exit(1);
    }

    // Sync legacy config with new config service during transition
    auto& settings = config_service->get_settings();
    cheat_config::aimbot_enabled = settings.aimbot.enabled;
    cheat_config::aimbot_silent_enabled = settings.aimbot.silent;
    cheat_config::aimbot_fov = settings.aimbot.fov;
    cheat_config::aimbot_smooth_factor = settings.aimbot.smooth_factor;
    cheat_config::no_animations = settings.misc.no_animations;
    cheat_config::freeze_lobby = settings.misc.freeze_lobby;
    cheat_config::speed_multiplier = settings.misc.speed_multiplier;
    cheat_config::gunmodel_fov = settings.misc.gunmodel_fov;
    cheat_config::player_fov = settings.misc.player_fov;
    cheat_config::noflash_enabled = settings.misc.noflash_enabled;
    cheat_config::glow_red = settings.glow.red;
    cheat_config::glow_green = settings.glow.green;
    cheat_config::glow_blue = settings.glow.blue;
    cheat_config::glow_alpha = settings.glow.alpha;
    cheat_config::glow_distance = settings.glow.distance;
    cheat_config::glow_opacity = settings.glow.opacity;

	if (!memory_service->initialize()) {
        LOG_P0("Critical failure during memory service initialization.");
		system("pause");
		exit(1);
	}

    if (!health_service->initialize()) {
        LOG_P1("Failed to initialize health service.");
    }

	if (!memory_service->is_kernel_mode()) {
        LOG_INFO("User-mode interface initialized. Access level will be determined upon attachment.");
	}
	else {
        LOG_INFO("Kernel-mode driver interface initialized (Full Access).");
	}

	overlay::enable();

    LOG_INFO("Looking for RainbowSix.exe...");
    context->set_state(core::CheatState::WaitingForProcess);
	while (!util::is_game_open("Rainbow Six", "R6Game", "RainbowSix.exe")) {
		if (overlay::input::key_pressed(VK_DELETE))
			exit(0);
		
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}

	uint32_t game_pid = util::get_pid_from_file("RainbowSix.exe");
    context->set_game_pid(game_pid);

	if (game_pid == 0) {
        LOG_ERROR("Invalid process id.");
		system("pause");
		exit(1);
	}

	if (!memory_service->is_kernel_mode()) {
		if (memory_service->has_write_access()) {
            LOG_INFO("Attached with FULL ACCESS (Read/Write).");
		}
		else {
            LOG_WARN("Attached with LIMITED ACCESS (Read-Only).");
            LOG_WARN("Write-based features (No Recoil, Glow, etc.) will be disabled.");
		}
	}
	
    LOG_INFO("Found PID: {}", game_pid);

	if (!memory_service->attach(game_pid)) {
        LOG_ERROR("Failed to attach to process memory.");
		system("pause");
		exit(1);
	}

    uintptr_t module_base = memory_service->get_module_base(L"RainbowSix.exe");
	if (module_base == 0) {
		module_base = memory_service->get_module_base(L"RainbowSix_Vulkan.exe");
	}
    context->set_module_base(module_base);

	if (module_base == 0) {
        LOG_ERROR("Failed to get module base address.");
		system("pause");
		exit(1);
	}

    LOG_INFO("Module base: 0x{:x}", module_base);

	Beep(500, 500);

    LOG_INFO("Updating game addresses...");
    // Note: game::update_addresses still uses globals for now, will refactor in later steps
	while (!game::update_addresses()) {
		std::this_thread::sleep_for(std::chrono::milliseconds(500));
        LOG_DEBUG("Still waiting for game addresses to update...");
	}
    LOG_INFO("Game addresses updated.");
	
    context->set_state(core::CheatState::Attached);

    // Watchdog Thread for Enterprise-Grade Auto-Recovery
    std::thread watchdog_thread([health_service, context]() {
        LOG_INFO("Watchdog thread started.");
        while (context->get_state() != core::CheatState::Detached) {
            auto health = health_service->check_health();
            if (health == core::SystemHealth::Critical) {
                LOG_P0("Watchdog detected CRITICAL system failure!");
                health_service->perform_auto_recovery();
                break;
            }
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        LOG_INFO("Watchdog thread exiting.");
    });
    watchdog_thread.detach();

    LOG_INFO("Cheat loop started. Press DELETE to exit.");
	while (overlay::input::key_pressed(VK_DELETE) == 0) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		
        if (context->get_state() == core::CheatState::Detached) {
            LOG_P1("Cheat loop terminated due to system detachment.");
            break;
        }

        core::PerformanceMetrics::instance().start_frame();

        // Frame-based cache invalidation for performance and consistency
        memory_service->clear_cache();
        memory_service->enable_caching(true);

		if (!game::in_match() || game::get_profile() == 0) {
            if (context->get_state() != core::CheatState::Detached)
                context->set_state(core::CheatState::Attached);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
            core::PerformanceMetrics::instance().end_frame();
			continue;
		}

        if (context->get_state() != core::CheatState::Detached)
            context->set_state(core::CheatState::InMatch);

        {
            core::ScopedTimer timer("cheat_loop");
		    cheat::run();
        }

        core::PerformanceMetrics::instance().end_frame();
	}

	cheat::restore();

	overlay::disable();

	Beep(500, 500);
	
    core::Logger::shutdown();
	exit(0);
}

int main() {
    RunCheat();
    return 0;
}
