#include <gtest/gtest.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

std::string testDataPath;

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  if (argc > 1) {
    testDataPath = argv[1];
  } else {
    throw std::runtime_error("No data file path given");
  }

  auto Logger = spdlog::stderr_color_mt("LOG");
  Logger->set_level(spdlog::level::trace);

  return RUN_ALL_TESTS();
}
