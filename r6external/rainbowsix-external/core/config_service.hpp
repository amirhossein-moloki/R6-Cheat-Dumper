#ifndef _CORE_CONFIG_SERVICE_HPP_
#define _CORE_CONFIG_SERVICE_HPP_

#include <string>
#include <nlohmann/json.hpp>
#include "iservice.hpp"

namespace core {
    struct AimbotSettings {
        bool enabled = true;
        bool silent = false;
        float fov = 100.0f;
        float smooth_factor = 40.0f;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(AimbotSettings, enabled, silent, fov, smooth_factor)
    };

    struct MiscSettings {
        bool no_animations = true;
        bool freeze_lobby = false;
        float speed_multiplier = 1.18f;
        float gunmodel_fov = 0.8726646304f;
        float player_fov = 1.570796371f;
        bool noflash_enabled = true;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(MiscSettings, no_animations, freeze_lobby, speed_multiplier, gunmodel_fov, player_fov, noflash_enabled)
    };

    struct GlowSettings {
        float red = 0.0f;
        float green = 0.25f;
        float blue = 1.0f;
        float alpha = 1.0f;
        float distance = 1.0f;
        float opacity = -1.0f;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(GlowSettings, red, green, blue, alpha, distance, opacity)
    };

    struct ConfigSettings {
        AimbotSettings aimbot;
        MiscSettings misc;
        GlowSettings glow;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(ConfigSettings, aimbot, misc, glow)
    };

    class ConfigService : public IService {
    public:
        ConfigService();
        ~ConfigService() override = default;

        bool initialize() override;
        void shutdown() override;
        std::string get_name() const override { return "ConfigService"; }

        bool load(const std::string& filename = "config.json");
        bool save(const std::string& filename = "config.json");

        ConfigSettings& get_settings() { return m_settings; }
        const ConfigSettings& get_settings() const { return m_settings; }

    private:
        ConfigSettings m_settings;
    };
}

#endif
