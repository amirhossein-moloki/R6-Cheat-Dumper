#ifndef _CORE_HEALTH_SERVICE_HPP_
#define _CORE_HEALTH_SERVICE_HPP_

#include <memory>
#include <atomic>
#include "iservice.hpp"

namespace core {
    class CheatContext;

    enum class SystemHealth {
        Healthy,
        Degraded,
        Critical,
        Unknown
    };

    class HealthService : public IService {
    public:
        HealthService(std::shared_ptr<CheatContext> context);
        ~HealthService();

        bool initialize() override;
        void shutdown() override;
        std::string get_name() const override { return "HealthService"; }

        SystemHealth check_health();
        bool is_game_running();
        bool is_driver_responding();

        void perform_auto_recovery();

    private:
        std::weak_ptr<CheatContext> m_context;
        std::atomic<bool> m_running;
    };
}

#endif
