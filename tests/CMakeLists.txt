cmake_minimum_required(VERSION 3.20)
project(technical-suite-tests)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(googletest)

add_executable(unit_tests
    test_config.cpp
    test_memory.cpp
    ../r6external/rainbowsix-external/core/config_service.cpp
    ../r6external/rainbowsix-external/core/logger.cpp
)

target_link_libraries(unit_tests
    GTest::gtest_main
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

target_include_directories(unit_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../r6external/rainbowsix-external
)

if(MSVC)
    target_compile_definitions(unit_tests PRIVATE _AMD64_)
endif()

add_test(NAME AllUnitTests COMMAND unit_tests)
