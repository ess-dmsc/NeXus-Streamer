project(isis_nexus_streamer)

set( SRC_FILES
        src/NexusFileReader.cpp)

set( INC_FILES
        include/NexusFileReader.h
        include/FileReader.h)

set( TEST_FILES
        test/NexusFileReaderTest.cpp
        test/HDF5FileTestHelpers.cpp
        test/HDF5FileTestHelpers.h)

#####################
## Libraries       ##
#####################

include_directories(
        CONAN_PKG::hdf5
        CONAN_PKG::h5cpp
        ${PROJECT_SOURCE_DIR}/include)

add_library(nexusFileReader_lib ${SRC_FILES} ${INC_FILES})

target_link_libraries(nexusFileReader_lib
        eventdata_lib
        CONAN_PKG::hdf5
        CONAN_PKG::h5cpp
        CONAN_PKG::spdlog)

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
