cmake_minimum_required(VERSION 3.2)
project(NeXus-Streamer)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pedantic-errors -Werror")
set(CMAKE_CXX_STANDARD 14)

set(SRC_CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH} ${SRC_CMAKE_MODULE_PATH})

if(CMAKE_COMPILER_IS_GNUCXX AND COV)
    include(CodeCoverage)
    set(path_to_test_data ${CMAKE_SOURCE_DIR}/data/)
    setup_target_for_coverage(coverage UnitTests coverage "-d;${path_to_test_data}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_COVERAGE}")
endif()

# Use gold linker if available on system (it is faster than default GNU linker)
if (UNIX AND NOT APPLE)
  execute_process(COMMAND ${CMAKE_C_COMPILER} -fuse-ld=gold -Wl,--version ERROR_QUIET OUTPUT_VARIABLE ld_version)
  if ("${ld_version}" MATCHES "GNU gold")
    message(STATUS "Using gold linker")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -fuse-ld=gold -Wl,--disable-new-dtags")
  endif()
endif()

set(CONAN "AUTO" CACHE STRING "conan options AUTO (conan must be in path) or MANUAL (expects conanbuildinfo.cmake in build directory)")
set(CONAN_PROFILE "default" CACHE STRING "Name of conan profile to use, uses default by default")
include(${CMAKE_MODULE_PATH}/conan.cmake)
if(${CONAN} MATCHES "AUTO")
    conan_cmake_run(CONANFILE conan/conanfile.txt
            PROFILE ${CONAN_PROFILE}
            BASIC_SETUP CMAKE_TARGETS
            BUILD outdated
            KEEP_RPATHS)
elseif(${CONAN} MATCHES "MANUAL")
    if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
        include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
        conan_basic_setup(TARGETS)
    else()
        message(FATAL_ERROR "CONAN set to MANUAL but no file named conanbuildinfo.cmake found in build directory")
    endif()
else()
    message(FATAL_ERROR "Unrecognised option for CONAN, use AUTO or MANUAL")
endif()

find_package(Gperftools)

#####################
## Versioning      ##
#####################

include(${SRC_CMAKE_MODULE_PATH}/Versioning.cmake)
set_version()
set(VERSION_INCLUDE_DIR ${CMAKE_BINARY_DIR}/version)
create_version_header(
        ${SRC_CMAKE_MODULE_PATH}/templates/Version.h.in
        ${VERSION_INCLUDE_DIR}/Version.h
)


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
    message("Linking all executables with tcmalloc")
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

add_subdirectory("${PROJECT_SOURCE_DIR}/serialisation")
add_subdirectory("${PROJECT_SOURCE_DIR}/nexus_producer")
add_subdirectory("${PROJECT_SOURCE_DIR}/nexus_file_reader")

####################
## Unit tests     ##
####################

include_directories(
        ${PROJECT_SOURCE_DIR}/nexus_producer/include
        ${PROJECT_SOURCE_DIR}/serialisation/include
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/include
        ${PROJECT_SOURCE_DIR}/core/include)

file(GLOB TEST_SRC_FILES
        ${PROJECT_SOURCE_DIR}/nexus_producer/test/*Test.cpp
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/test/*Test.cpp
        ${PROJECT_SOURCE_DIR}/serialisation/test/*Test.cpp
        unitTestRunner.cpp)

add_executable(UnitTests ${TEST_SRC_FILES})

target_link_libraries(UnitTests
        CONAN_PKG::CLI11
        producerUnitTests
        eventDataUnitTests
        fileReaderUnitTests)

# Make the tests available to run through ctest
# Do "ctest -VV" in the build directory
add_test(NAME all_tests COMMAND UnitTests "${PROJECT_SOURCE_DIR}/data/")
