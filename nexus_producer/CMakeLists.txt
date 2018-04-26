project(nexus-publisher)

set( SRC_FILES  src/NexusPublisher.cpp)

set( INC_FILES  include/EventPublisher.h
                include/NexusPublisher.h)

set( TEST_FILES test/NexusPublisherTest.cpp)

#####################
## Libraries       ##
#####################

include_directories(
        CONAN_PKG::librdkafka
        CONAN_PKG::hdf5
        CONAN_PKG::cli11
        ${PROJECT_SOURCE_DIR}/include)

add_library(nexusPublisher_lib ${PROJECT_SOURCE_DIR}/src/NexusPublisher.cpp)

add_library(eventPublisher_lib
        ${PROJECT_SOURCE_DIR}/src/KafkaEventPublisher.cpp)

target_link_libraries(eventPublisher_lib
        CONAN_PKG::librdkafka)
target_link_libraries(nexusPublisher_lib
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

set(publisher-target main_nexusPublisher)
add_executable(${publisher-target} ${PROJECT_SOURCE_DIR}/src/main.cpp)

target_link_libraries(${publisher-target}
        CONAN_PKG::cli11
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
        ${TEST_FILES})
target_link_libraries(producerUnitTests ${tests_LINK_LIBRARIES})
