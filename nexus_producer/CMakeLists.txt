project(nexus-publisher)

set( SRC_FILES  src/NexusPublisher.cpp)

set( INC_FILES  include/EventPublisher.h
                include/NexusPublisher.h
                include/OptionalArgs.h)

set( TEST_FILES test/NexusPublisherTest.cpp)

#####################
## Libraries       ##
#####################

include_directories(
        ${PROJECT_SOURCE_DIR}/include)

add_library(nexusPublisher_lib ${SRC_FILES} ${INC_FILES})

add_library(eventPublisher_lib
        src/KafkaEventPublisher.cpp include/KafkaEventPublisher.h)

target_link_libraries(eventPublisher_lib
        core_lib
        CONAN_PKG::librdkafka
        CONAN_PKG::spdlog)
target_link_libraries(nexusPublisher_lib
        CONAN_PKG::spdlog
        core_lib
        nexusFileReader_lib
        eventdata_lib)

set(core_libraries
        eventdata_lib
        nexusFileReader_lib
        nexusPublisher_lib
        eventPublisher_lib)

#####################
## Main executable ##
#####################

set(publisher-target nexus-streamer)
add_executable(${publisher-target} ${PROJECT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(${publisher-target}
        CONAN_PKG::cli11
        CONAN_PKG::spdlog
        ${core_libraries})

#####################
## Unit Tests      ##
#####################

set(tests_LINK_LIBRARIES
        CONAN_PKG::gtest
        ${core_libraries})
if(UNIX)
    list(APPEND tests_LINK_LIBRARIES pthread)
endif()

add_library(producerUnitTests
        ${TEST_FILES} include/MockEventPublisher.h)
target_link_libraries(producerUnitTests ${tests_LINK_LIBRARIES})
