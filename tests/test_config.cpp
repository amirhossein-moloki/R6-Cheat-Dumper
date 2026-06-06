#include <gtest/gtest.h>
#include "core/config_service.hpp"
#include <fstream>

class ConfigServiceTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a dummy config for testing
        std::ofstream file("test_config.json");
        file << R"({
            "aimbot": { "enabled": false, "silent": true, "fov": 90.0, "smooth_factor": 10.0 },
            "misc": { "no_animations": false, "freeze_lobby": true, "speed_multiplier": 1.5, "gunmodel_fov": 1.0, "player_fov": 1.2, "noflash_enabled": false },
            "glow": { "red": 1.0, "green": 0.0, "blue": 0.0, "alpha": 0.5, "distance": 10.0, "opacity": 0.8 }
        })";
        file.close();
    }

    void TearDown() override {
        std::remove("test_config.json");
    }
};

TEST_F(ConfigServiceTest, LoadConfig) {
    core::ConfigService config;
    EXPECT_TRUE(config.load("test_config.json"));

    auto& settings = config.get_settings();
    EXPECT_FALSE(settings.aimbot.enabled);
    EXPECT_TRUE(settings.aimbot.silent);
    EXPECT_NEAR(settings.aimbot.fov, 90.0f, 0.01f);
    EXPECT_TRUE(settings.misc.freeze_lobby);
    EXPECT_NEAR(settings.glow.red, 1.0f, 0.01f);
}

TEST_F(ConfigServiceTest, DefaultConfig) {
    core::ConfigService config;
    // Should fail to load non-existent file and use defaults
    EXPECT_FALSE(config.load("non_existent.json"));

    auto& settings = config.get_settings();
    EXPECT_TRUE(settings.aimbot.enabled); // Default is true
}
