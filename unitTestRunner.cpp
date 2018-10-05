#include <gtest/gtest.h>

std::string testDataPath;

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  if (argc > 1) {
    testDataPath = argv[1];
  } else {
    throw std::runtime_error("No data file path given");
  }
  return RUN_ALL_TESTS();
}
