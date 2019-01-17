#include <CLI/CLI.hpp>
#include <gtest/gtest.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

std::string testDataPath;

int main(int argc, char *argv[]) {
  CLI::App App{"Running unit tests for NeXus-Streamer"};

  App.add_option("-d,--data-path", testDataPath,
                 "Full path to test data directory")
      ->required()
      ->check(CLI::ExistingDirectory);

  ::testing::InitGoogleTest(&argc, argv);

  CLI11_PARSE(App, argc, argv);

  auto Logger = spdlog::stderr_color_mt("LOG");
  Logger->set_level(spdlog::level::trace);

  return RUN_ALL_TESTS();
}
