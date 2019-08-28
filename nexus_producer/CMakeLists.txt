project(nexus-publisher)

set( SRC_FILES
        src/NexusPublisher.cpp
        src/Timer.cpp
        src/JSONDescriptionLoader.cpp)

set( INC_FILES
        include/Publisher.h
        include/NexusPublisher.h
        ../core/include/OptionalArgs.h
        include/Timer.h
        include/JSONDescriptionLoader.h
        include/TopicNames.h)

set( TEST_FILES
        test/NexusPublisherTest.cpp
        test/TimerTest.cpp
        test/JSONDescriptionLoaderTest.cpp)

#####################
## Libraries       ##
#####################

include_directories(
        ${PROJECT_SOURCE_DIR}/include)

add_library(nexusPublisher_lib ${SRC_FILES} ${INC_FILES})
target_link_libraries(nexusPublisher_lib
        CONAN_PKG::spdlog
        CONAN_PKG::flatbuffers
        CONAN_PKG::jsonformoderncpp
        nexusFileReader_lib
        serialisation_lib)
target_include_directories(nexusPublisher_lib PUBLIC ${PROJECT_SOURCE_DIR}/core/include)

add_library(eventPublisher_lib
        src/KafkaPublisher.cpp include/KafkaPublisher.h)
target_link_libraries(eventPublisher_lib
        CONAN_PKG::librdkafka
        CONAN_PKG::spdlog
        CONAN_PKG::flatbuffers)
target_include_directories(eventPublisher_lib PUBLIC ${PROJECT_SOURCE_DIR}/core/include)


set(core_libraries
        serialisation_lib
        nexusFileReader_lib
        nexusPublisher_lib
        eventPublisher_lib)

#####################
## Main executable ##
#####################

set(publisher-target nexus-streamer)
add_executable(${publisher-target} ${PROJECT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(${publisher-target}
        CONAN_PKG::CLI11
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
        ${TEST_FILES} include/MockPublisher.h)
target_link_libraries(producerUnitTests ${tests_LINK_LIBRARIES})
