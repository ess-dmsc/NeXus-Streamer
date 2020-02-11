#include <random>
#include <sstream>

uint32_t random_character() {
  std::random_device randomDevice;
  std::mt19937 generator(randomDevice());
  std::uniform_int_distribution<> distribution(0, 255);
  return distribution(generator);
}

std::string generate_uuid(const size_t length) {
  std::stringstream ss;
  for (size_t i = 0; i < length; i++) {
    std::stringstream hexstream;
    hexstream << std::hex << random_character();
    auto hex_string = hexstream.str();
    ss << (hex_string.length() < 2 ? '0' + hex_string : hex_string);
  }
  return ss.str();
}
