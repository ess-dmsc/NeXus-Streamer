project(isis_nexus_streamer)

set( SRC_FILES
        src/NexusFileReader.cpp)

set( INC_FILES
        include/NexusFileReader.h)

set( TEST_FILES
        test/NexusFileReaderTest.cpp)

#####################
## Libraries       ##
#####################

include_directories(
        CONAN_PKG::hdf5
        ${PROJECT_SOURCE_DIR}/include)

add_library(nexusFileReader_lib ${SRC_FILES} ${INC_FILES})

target_link_libraries(nexusFileReader_lib
        eventdata_lib
        CONAN_PKG::hdf5)

#####################
## Unit Tests      ##
#####################

set(tests_LINK_LIBRARIES
        CONAN_PKG::gtest
        nexusFileReader_lib)
if(UNIX)
    list(APPEND tests_LINK_LIBRARIES pthread)
endif()

add_library(fileReaderUnitTests
        ${TEST_FILES})
target_link_libraries(fileReaderUnitTests ${tests_LINK_LIBRARIES})
