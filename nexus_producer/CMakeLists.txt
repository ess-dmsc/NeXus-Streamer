project(isis_nexus_streamer)

set( SRC_FILES
        src/NexusPublisher.cpp)

set( INC_FILES
        include/EventPublisher.h
        include/NexusPublisher.h)

set( TEST_FILES
        test/NexusPublisherTest.cpp)

#####################
## Libraries       ##
#####################

include_directories(
        ${LibRDKafka_INCLUDE_DIR}
        ${HDF5_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR}/include)

add_library(nexusPublisher_lib ${PROJECT_SOURCE_DIR}/src/NexusPublisher.cpp)
add_library(eventPublisher_lib
        ${PROJECT_SOURCE_DIR}/src/KafkaEventPublisher.cpp)

target_link_libraries(eventPublisher_lib
        ${LibRDKafka_LIBRARIES})
target_link_libraries(nexusPublisher_lib
        nexusFileReader_lib
        eventdata_lib
        rundata_lib)

set(core_libraries
        eventdata_lib
        nexusFileReader_lib
        nexusPublisher_lib
        eventPublisher_lib)

#####################
## Main executable ##
#####################

add_executable(main_nexusPublisher ${PROJECT_SOURCE_DIR}/src/main.cpp)
target_link_libraries(main_nexusPublisher
        ${core_libraries})

#####################
## Unit Tests      ##
#####################

set(tests_LINK_LIBRARIES
        gtest
        gmock
        ${core_libraries})
if(UNIX)
    list(APPEND tests_LINK_LIBRARIES pthread)
endif()

add_library(producerUnitTests
        ${TEST_FILES})
target_link_libraries(producerUnitTests ${tests_LINK_LIBRARIES})

#####################
## Test Coverage   ##
#####################

# Add all src and inc files to test coverage check
if (COVERALLS)
    foreach( loop_var ${SRC_FILES} ${INC_FILES})
        set_property(GLOBAL APPEND PROPERTY COVERAGE_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}")
    endforeach(loop_var)
endif()
