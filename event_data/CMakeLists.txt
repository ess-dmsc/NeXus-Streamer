project(isis_nexus_streamer)

set( SRC_FILES
        src/EventData.cpp
        src/RunData.cpp
        src/DetectorSpectrumMapData.cpp)

set( INC_FILES
        include/EventData.h
        include/RunData.h
        include/DetectorSpectrumMapData.h)

set( TEST_FILES
        test/EventDataTest.cpp
        test/RunDataTest.cpp
        test/DetectorSpectrumMapDataTest.cpp)

#####################
## Libraries       ##
#####################

include_directories(
        ${PROJECT_SOURCE_DIR}/include)

add_library(eventdata_lib ${PROJECT_SOURCE_DIR}/src/EventData.cpp)
add_library(rundata_lib ${PROJECT_SOURCE_DIR}/src/RunData.cpp)
add_library(detSpecMapData_lib ${PROJECT_SOURCE_DIR}/src/DetectorSpectrumMapData.cpp)

#####################
## Unit Tests      ##
#####################

set(eventData_tests_LINK_LIBRARIES
        gtest
        gmock
        eventdata_lib
        rundata_lib
        detSpecMapData_lib)
if(UNIX)
    list(APPEND tests_LINK_LIBRARIES pthread)
endif()

add_library(eventDataUnitTests
        ${TEST_FILES})
target_link_libraries(eventDataUnitTests ${eventData_tests_LINK_LIBRARIES})

#####################
## Test Coverage   ##
#####################

# Add all src and inc files to test coverage check
if (COVERALLS)
    foreach( loop_var ${SRC_FILES} ${INC_FILES})
        set_property(GLOBAL APPEND PROPERTY COVERAGE_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/${loop_var}")
    endforeach(loop_var)
endif()
