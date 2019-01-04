project(core)

add_library(core_lib include/Message.h)
set_target_properties(core_lib PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(core_lib CONAN_PKG::FlatBuffers)
