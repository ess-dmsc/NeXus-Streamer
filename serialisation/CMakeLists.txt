project(isis_nexus_streamer)

set( SRC_FILES
        src/EventData.cpp
        src/HistogramData.cpp
        src/RunData.cpp
        src/DetectorSpectrumMapData.cpp
        src/SampleEnvironmentEvent.cpp
        src/SampleEnvironmentEventInt.cpp
        src/SampleEnvironmentEventLong.cpp
        src/SampleEnvironmentEventDouble.cpp
        )

set( INC_FILES
        include/EventData.h
        include/HistogramData.h
        include/RunData.h
        include/DetectorSpectrumMapData.h
        include/SampleEnvironmentEvent.h
        include/SampleEnvironmentEventInt.h
        include/SampleEnvironmentEventLong.h
        include/SampleEnvironmentEventDouble.h
        )

set( TEST_FILES
        test/EventDataTest.cpp
        test/HistogramDataTest.cpp
        test/RunDataTest.cpp
        test/DetectorSpectrumMapDataTest.cpp
        test/SampleEnvironmentEventTest.cpp)

#####################
## Libraries       ##
#####################

include_directories(
        ${PROJECT_SOURCE_DIR}/include)

add_library(serialisation_lib ${SRC_FILES} ${INC_FILES})
target_include_directories(serialisation_lib PUBLIC ${PROJECT_SOURCE_DIR}/core/include)
target_link_libraries(serialisation_lib
        CONAN_PKG::flatbuffers
        CONAN_PKG::streaming-data-types)

#####################
## Unit Tests      ##
#####################

set(eventData_tests_LINK_LIBRARIES
        CONAN_PKG::gtest
        serialisation_lib)
if(UNIX)
    list(APPEND tests_LINK_LIBRARIES pthread)
endif()

add_library(eventDataUnitTests
        ${TEST_FILES})
target_link_libraries(eventDataUnitTests ${eventData_tests_LINK_LIBRARIES})

######################
## Benchmark        ##
######################

add_executable(benchmark_serialisation test/BenchmarkSerialisation.cpp)
target_link_libraries(benchmark_serialisation CONAN_PKG::google-benchmark serialisation_lib)
if(WIN32)
    target_link_libraries(benchmark_serialisation shlwapi.lib)
endif(WIN32)
