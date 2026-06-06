#include "logger.hpp"
#include <vector>

namespace core {
    std::shared_ptr<spdlog::logger> Logger::s_logger;

    void Logger::initialize(const std::string& log_file) {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[%H:%M:%S] [%^%l%$] %v");

        auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(log_file, 1024 * 1024 * 10, 5);
        file_sink->set_level(spdlog::level::trace);
        file_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] [%t] %v");

        std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
        s_logger = std::make_shared<spdlog::logger>("tech_suite", sinks.begin(), sinks.end());
        s_logger->set_level(spdlog::level::trace);

        spdlog::register_logger(s_logger);
        spdlog::set_default_logger(s_logger);
        spdlog::flush_on(spdlog::level::info);
    }

    void Logger::shutdown() {
        spdlog::shutdown();
    }
}
