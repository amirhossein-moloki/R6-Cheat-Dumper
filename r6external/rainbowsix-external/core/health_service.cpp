#include "health_service.hpp"
#include "cheat_context.hpp"
#include "logger.hpp"
#include "../util/util.hpp"

namespace core {
    HealthService::HealthService(std::shared_ptr<CheatContext> context)
        : m_context(context), m_running(false) {}

    HealthService::~HealthService() {
        shutdown();
    }

    bool HealthService::initialize() {
        m_running = true;
        LOG_INFO("HealthService initialized.");
        return true;
    }

    void HealthService::shutdown() {
        m_running = false;
    }

    SystemHealth HealthService::check_health() {
        auto ctx = m_context.lock();
        if (!ctx) return SystemHealth::Unknown;

        if (!is_game_running()) {
            return SystemHealth::Critical;
        }

        auto memory_service = ctx->get_memory_service();
        if (!memory_service || !memory_service->is_attached()) {
            return SystemHealth::Degraded;
        }

        return SystemHealth::Healthy;
    }

    bool HealthService::is_game_running() {
        return util::is_game_open("Rainbow Six", "R6Game", "RainbowSix.exe");
    }

    bool HealthService::is_driver_responding() {
        auto ctx = m_context.lock();
        if (!ctx) return false;

        auto memory_service = ctx->get_memory_service();
        return memory_service && memory_service->is_kernel_mode();
    }

    void HealthService::perform_auto_recovery() {
        LOG_WARN("Executing auto-recovery sequence...");
        auto ctx = m_context.lock();
        if (!ctx) return;

        ctx->set_state(CheatState::Detached);
        auto memory_service = ctx->get_memory_service();
        if (memory_service) {
            memory_service->clear_cache();
        }

        LOG_INFO("Auto-recovery complete. System returned to safe state.");
    }
}
