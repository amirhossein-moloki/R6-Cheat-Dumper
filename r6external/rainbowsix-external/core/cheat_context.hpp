#ifndef _CORE_CHEAT_CONTEXT_HPP_
#define _CORE_CHEAT_CONTEXT_HPP_

#include <memory>
#include <map>
#include <string>
#include <atomic>
#include "imemory_service.hpp"
#include "../util/math/vector.hpp"

namespace core {
    class ConfigService;
    class HealthService;

    enum class CheatState {
        WaitingForProcess,
        Attached,
        InMatch,
        Detached
    };

    class CheatContext {
    public:
        CheatContext() : m_state(CheatState::WaitingForProcess), m_game_pid(0), m_module_base(0) {
            s_instance = this;
        }

        static CheatContext* get_instance() { return s_instance; }

        void set_memory_service(std::shared_ptr<IMemoryService> service) { m_memory_service = service; }
        std::shared_ptr<IMemoryService> get_memory_service() const { return m_memory_service; }

        void set_config_service(std::shared_ptr<ConfigService> service) { m_config_service = service; }
        std::shared_ptr<ConfigService> get_config_service() const { return m_config_service; }

        void set_health_service(std::shared_ptr<HealthService> service) { m_health_service = service; }
        std::shared_ptr<HealthService> get_health_service() const { return m_health_service; }

        CheatState get_state() const { return m_state; }
        void set_state(CheatState state) { m_state = state; }

        uint32_t get_game_pid() const { return m_game_pid; }
        void set_game_pid(uint32_t pid) { m_game_pid = pid; }

        uintptr_t get_module_base() const { return m_module_base; }
        void set_module_base(uintptr_t base) { m_module_base = base; }

        // Game addresses (migrated from globals)
        struct {
            uintptr_t game_manager = 0;
            uintptr_t game_profile = 0;
            uintptr_t network_manager = 0;
            uintptr_t input_manager = 0;
            uintptr_t round_manager = 0;
            uintptr_t glow_manager = 0;
            uintptr_t fov_manager = 0;
            uintptr_t freeze_manager = 0;
            uintptr_t entity_list = 0;
            uintptr_t render = 0;
            uintptr_t game_render = 0;
            uintptr_t engine_link = 0;
            uintptr_t engine = 0;
            uintptr_t camera = 0;
            uintptr_t vtable = 0;
        } addresses;

        struct {
            int window_horizontal_size = 0;
            int window_vertical_size = 0;
            vec3_t camera_view_right;
            vec3_t camera_view_up;
            vec3_t camera_view_forward;
            vec3_t camera_view_translation;
            float camera_view_fovx = 0;
            float camera_view_fovy = 0;
        } camera_state;

        bool w2s_good = false;
        uintptr_t targeted_entity = 0;

    private:
        static CheatContext* s_instance;
        std::atomic<CheatState> m_state;
        std::shared_ptr<IMemoryService> m_memory_service;
        std::shared_ptr<ConfigService> m_config_service;
        std::shared_ptr<HealthService> m_health_service;
        uint32_t m_game_pid;
        uintptr_t m_module_base;
    };
}

#endif
