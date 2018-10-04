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
SET(CONAN "AUTO" CACHE STRING "conan options AUTO (conan must be in path), MANUAL (expects conanbuildinfo.cmake in build directory) or DISABLE")

if(${CONAN} MATCHES "AUTO")
    include(${CMAKE_MODULE_PATH}/conan.cmake)
    conan_cmake_run(CONANFILE conan/conanfile.txt
            PROFILE ${CONAN_PROFILE}
            BASIC_SETUP CMAKE_TARGETS
            BUILD_TYPE "None"
            BUILD outdated)
elseif(${CONAN} MATCHES "MANUAL")
    if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
        include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
        conan_basic_setup(TARGETS)
    else()
        MESSAGE(FATAL_ERROR "CONAN set to MANUAL but no file named conanbuildinfo.cmake found in build directory")
    endif()
elseif(NOT ${CONAN} MATCHES "DISABLE")
    MESSAGE(FATAL_ERROR "Unrecognised option for CONAN, use AUTO, MANUAL or DISABLE")
endif()

include(CTest)
enable_testing()

find_package(Gperftools)
find_package(h5cpp REQUIRED)

#####################
## Doxygen         ##
#####################

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
if(DOXYGEN_FOUND AND BUILD_DOCS)
    configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
    add_custom_target(doc
            ${DOXYGEN_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen" VERBATIM
            )
endif()

#####################
## TCMalloc        ##
#####################
# if tcmalloc is available then link all executables against it
if (GPERFTOOLS_TCMALLOC AND NOT BUILD_DOCS)
    MESSAGE("Linking all executables with tcmalloc")
    macro (add_executable _name)
        # invoke built-in add_executable
        _add_executable(${ARGV})
        if (PROFILE)
            if (TARGET ${_name})
                target_link_libraries(${_name} ${GPERFTOOLS_TCMALLOC_AND_PROFILER})
            endif()
         else ()
            if (TARGET ${_name})
                target_link_libraries(${_name} ${GPERFTOOLS_TCMALLOC})
            endif()
        endif()
    endmacro()
endif()


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
        CONAN_PKG::h5cpp
        ${PROJECT_SOURCE_DIR}/nexus_producer/include
        ${PROJECT_SOURCE_DIR}/event_data/include
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/include)

file(GLOB TEST_SRC_FILES
        ${PROJECT_SOURCE_DIR}/nexus_producer/test/*Test.cpp
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/test/*Test.cpp
        ${PROJECT_SOURCE_DIR}/event_data/test/*Test.cpp
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
