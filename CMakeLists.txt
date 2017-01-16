cmake_minimum_required(VERSION 2.8)
project(isis_nexus_streamer_for_mantid)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pedantic-errors")

if(MSVC)
   # Git LFS does not work properly with <= 1.9
  #find_package ( Git 1.9.5 REQUIRED )
  #find_package ( GitLFS REQUIRED)

  # Use ExternalProject functionality as it already knows how to do clone/update
  include ( ExternalProject )
  set( EXTERNAL_ROOT ${PROJECT_SOURCE_DIR}/external )
  set( THIRD_PARTY_GIT_URL "https://github.com/ScreamingUdder/thirdparty-msvc.git" )
  set ( THIRD_PARTY_GIT_SHA1 master)
  set ( THIRD_PARTY_DIR ${EXTERNAL_ROOT}/src/ThirdParty)
  # Generates a script to do the clone/update in tmp
  set ( _project_name ThirdParty )
  ExternalProject_Add( ${_project_name}
    PREFIX ${EXTERNAL_ROOT}
    GIT_REPOSITORY ${THIRD_PARTY_GIT_URL}
    GIT_TAG ${THIRD_PARTY_GIT_SHA1}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    TEST_COMMAND ""
  )
  set_target_properties ( ${_project_name} PROPERTIES
                           EXCLUDE_FROM_DEFAULT_BUILD 1
                           EXCLUDE_FROM_ALL 1)

  # Do fetch/update now as we need the dependencies to configure
  set ( _tmp_dir ${EXTERNAL_ROOT}/tmp )
  if ( NOT EXISTS ${THIRD_PARTY_DIR}/.git )
    message ( STATUS "Fetching third party dependencies" )
    # As of git lfs 1.02 the default 'git checkout' behaviour is very slow for a large amount of data. Running the
    # 'git lfs fetch' command however produces better suitable performance as it downloads everything in parallel.
    # We there for first clone the bare repository containing the data pointers and update them manually
    # see https://github.com/github/git-lfs/issues/376 for more information
    set ( ENV{GIT_LFS_SKIP_SMUDGE} 1 )
    execute_process ( COMMAND ${CMAKE_COMMAND} ARGS -P ${_tmp_dir}/${_project_name}-gitclone.cmake
                      RESULT_VARIABLE error_code )
    if ( error_code )
      message(FATAL_ERROR "Failed to clone repository: '${THIRD_PARTY_GIT_URL}'")
    endif ()
    unset ( ENV{GIT_LFS_SKIP_SMUDGE} )
    # Fetch the binary data
    execute_process ( COMMAND ${GIT_EXECUTABLE} lfs fetch
                      WORKING_DIRECTORY ${THIRD_PARTY_DIR}
                      RESULT_VARIABLE error_code )
    if ( error_code )
      message(FATAL_ERROR "Failed to download third party binary data. Check your network connection")
    endif ()
    # Checkout the data from the index to the working directory
    execute_process ( COMMAND ${GIT_EXECUTABLE} lfs checkout
                      WORKING_DIRECTORY ${THIRD_PARTY_DIR}
                      RESULT_VARIABLE error_code )
  else ()
    message ( STATUS "Updating third party dependencies" )
    # Assume the updates are small & don't run git lfs fetch
    execute_process ( COMMAND ${CMAKE_COMMAND} ARGS -P ${_tmp_dir}/${_project_name}-gitupdate.cmake
                      RESULT_VARIABLE error_code )
    if ( error_code )
      message(FATAL_ERROR "Failed to update repository: '${THIRD_PARTY_GIT_URL}'")
    endif ()
  endif ()
  unset ( _tmp_dir )

  set ( THIRD_PARTY_BIN "${THIRD_PARTY_DIR}/bin;" )
  message ( STATUS "Third party dependencies are in ${THIRD_PARTY_DIR}" )
  # Add to the path so that cmake can configure correctly without the user having to do it
  set ( ENV{PATH} "${THIRD_PARTY_BIN};$ENV{PATH}" )

  # TreatWChar_tAsBuiltInType: true == /Zc:wchar_t  false == /Zc:wchar_t-
  set ( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-" )

  set(CMAKE_MODULE_PATH
         ${CMAKE_MODULE_PATH} ${THIRD_PARTY_DIR}/cmake)
endif()

set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(HDF5 COMPONENTS CXX HL REQUIRED)
find_package(LibRDKafka)
find_package(Tcmalloc)

include(CTest)
enable_testing()

# We probably don't want this to run on every build.
option(COVERALLS "Generate coveralls data" OFF)

if (COVERALLS)
    include(Coveralls)
    coveralls_turn_on_coverage()
endif()

#####################
## TCMalloc        ##
#####################

# if tcmalloc is available then link all executables against it
if (TCMALLOC_FOUND)
    macro (add_executable _name)
        # invoke built-in add_executable
        _add_executable(${ARGV})
        if (TARGET ${_name})
            target_link_libraries(${_name} tcmalloc_minimal)
        endif()
    endmacro()
endif()

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

#####################
## Get gtest       ##
#####################

# download and  build gtest and gmock
if (CMAKE_VERSION VERSION_LESS 3.2)
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "")
else()
    set(UPDATE_DISCONNECTED_IF_AVAILABLE "UPDATE_DISCONNECTED 1")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DownloadProject.cmake)
download_project(PROJ                googletest
        GIT_REPOSITORY      https://github.com/google/googletest.git
        GIT_TAG             master
        ${UPDATE_DISCONNECTED_IF_AVAILABLE})

# Prevent GoogleTest from overriding our compiler/linker options
# when building with Visual Studio
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR})

# When using CMake 2.8.11 or later, header path dependencies
# are automatically added to the gtest and gmock targets.
# For earlier CMake versions, we have to explicitly add the
# required directories to the header search path ourselves.
if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${gtest_SOURCE_DIR}/include"
            "${gmock_SOURCE_DIR}/include")
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
        ${HDF5_INCLUDE_DIRS}
        ${LibRDKafka_INCLUDE_DIR}
        ${PROJECT_SOURCE_DIR}/nexus_producer/include
        ${PROJECT_SOURCE_DIR}/event_data/include
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/include)

file(GLOB TEST_SRC_FILES
        ${PROJECT_SOURCE_DIR}/nexus_producer/test/*.cpp
        ${PROJECT_SOURCE_DIR}/event_data/test/*.cpp
        ${PROJECT_SOURCE_DIR}/nexus_file_reader/test/*.cpp
        unitTestRunner.cpp)

add_executable(unitTests ${TEST_SRC_FILES})

target_link_libraries(unitTests
        gtest
        gmock
        producerUnitTests
        eventDataUnitTests
        fileReaderUnitTests)

# Make the tests available to run through ctest
# Do "ctest -VV" in the build directory
add_test(NAME all_tests COMMAND unitTests "${PROJECT_SOURCE_DIR}/data/")

#####################
## Test coverage   ##
#####################

if (COVERALLS)
    get_property(ALL_SRCS GLOBAL PROPERTY COVERAGE_SRCS)
    set(SRCS_FILE "")
    foreach (SRC ${ALL_SRCS})
        set(SRCS_FILE "${SRCS_FILE}\n${SRC}")
    endforeach()
    #remove initial \n
    string(SUBSTRING ${SRCS_FILE} 1 -1 SRCS_FILE)
    set( SRCS_FILENAME "${CMAKE_CURRENT_BINARY_DIR}/sources.txt")
    file(WRITE ${SRCS_FILENAME} ${SRCS_FILE})
    coveralls_setup(
            "${SRCS_FILENAME}"
            ON
            "${PROJECT_SOURCE_DIR}/cmake")
endif ()
