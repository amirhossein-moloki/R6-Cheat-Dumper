#ifndef _LOGGER_HPP_
#define _LOGGER_HPP_

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <memory>

namespace logger {
    inline void initialize() {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::info);
        console_sink->set_pattern("[%^%l%$] %v");

        auto logger = std::make_shared<spdlog::logger>("dumper", console_sink);
        logger->set_level(spdlog::level::trace);

        spdlog::register_logger(logger);
        spdlog::set_default_logger(logger);
    }

    inline void shutdown() {
        spdlog::shutdown();
    }
}

#define LOG_TRACE(...) spdlog::trace(__VA_ARGS__)
#define LOG_DEBUG(...) spdlog::debug(__VA_ARGS__)
#define LOG_INFO(...)  spdlog::info(__VA_ARGS__)
#define LOG_WARN(...)  spdlog::warn(__VA_ARGS__)
#define LOG_ERROR(...) spdlog::error(__VA_ARGS__)

#endif
