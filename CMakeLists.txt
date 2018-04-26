cmake_minimum_required(VERSION 3.2)
project(NeXus-Streamer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic-errors")

set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(CMAKE_COMPILER_IS_GNUCXX AND COV)
    include(CodeCoverage)
    set(path_to_test_data ${CMAKE_SOURCE_DIR}/data/)
    setup_target_for_coverage(coverage UnitTests coverage "${path_to_test_data}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
endif()

set(CONAN_PROFILE "default" CACHE STRING "Name of conan profile to use, uses default by default")
include(${CMAKE_MODULE_PATH}/conan.cmake)
conan_cmake_run(CONANFILE conan/conanfile.txt
        PROFILE ${CONAN_PROFILE}
        BASIC_SETUP CMAKE_TARGETS
        BUILD_TYPE "None"
        BUILD outdated)

include(CTest)
enable_testing()

#####################
## Doxygen         ##
#####################

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
endif(DOXYGEN_FOUND)


####################
## Add subdirs    ##
####################

add_subdirectory("${PROJECT_SOURCE_DIR}/event_data")
add_subdirectory("${PROJECT_SOURCE_DIR}/nexus_producer")
add_subdirectory("${PROJECT_SOURCE_DIR}/nexus_file_reader")

####################
## Unit tests     ##
####################

include_directories(
        CONAN_PKG::h5cpp
        CONAN_PKG::librdkafka
        ${PROJECT_SOURCE_DIR}/nexus_producer/include
        ${PROJECT_SOURCE_DIR}/event_data/include
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/include)

file(GLOB TEST_SRC_FILES
        ${PROJECT_SOURCE_DIR}/nexus_producer/test/*.cpp
        ${PROJECT_SOURCE_DIR}/event_data/test/*.cpp
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/test/*.cpp
        unitTestRunner.cpp)

add_executable(UnitTests ${TEST_SRC_FILES})

target_link_libraries(UnitTests
        CONAN_PKG::gtest
        producerUnitTests
        eventDataUnitTests
        fileReaderUnitTests)

# Make the tests available to run through ctest
# Do "ctest -VV" in the build directory
add_test(NAME all_tests COMMAND UnitTests "${PROJECT_SOURCE_DIR}/data/")
