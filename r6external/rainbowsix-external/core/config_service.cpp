#include "config_service.hpp"
#include "logger.hpp"
#include <fstream>

namespace core {
    ConfigService::ConfigService() : m_settings{} {}

    bool ConfigService::initialize() {
        if (!load()) {
            LOG_INFO("No existing config found, creating default config.json");
            save();
        }
        return true;
    }

    void ConfigService::shutdown() {
        save();
    }

    bool ConfigService::load(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;

        try {
            nlohmann::json j;
            file >> j;
            m_settings = j.get<ConfigSettings>();
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Error loading config: {}", e.what());
            return false;
        }
    }

    bool ConfigService::save(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) return false;

        try {
            nlohmann::json j = m_settings;
            file << j.dump(4);
            return true;
        } catch (const std::exception& e) {
            LOG_ERROR("Error saving config: {}", e.what());
            return false;
        }
    }
}
