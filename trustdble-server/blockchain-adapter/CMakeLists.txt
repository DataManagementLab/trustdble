# Works with 3.16 and tested through 3.20
cmake_minimum_required(VERSION 3.16...3.20)

# Project name and a few useful settings. Other commands can pick up the results
project(
  TrustdbleAdapters
  VERSION 0.1
  DESCRIPTION "TrustDBle Adapters project"
  LANGUAGES CXX)

# Required for compilation under MacOS
set (CMAKE_CXX_STANDARD 17)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(FetchContent)
include(trustdble_configure)

add_subdirectory(interface)
add_subdirectory(fabric/adapter)
add_subdirectory(stub/adapter)
add_subdirectory(ethereum/adapter)
add_subdirectory(factory)
add_subdirectory(cli)
add_subdirectory(utils)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  # Docs only available if this is the main app
  find_package(Doxygen)
  if(Doxygen_FOUND)
    add_subdirectory(docs)
  else()
    message(STATUS "Doxygen not found, not building docs")
  endif()

  # Tests only available if this is the main app
  include(CTest)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.10.0
  )
  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW) # Required since googleTest uses an older cmake version
  FetchContent_MakeAvailable(googletest)
endif()

# Testing only available if this is the main app
# Emergency override MyProjectName_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR TrustDBleAdapters_BUILD_TESTING)
   AND BUILD_TESTING)
  include(GoogleTest)
  add_subdirectory(fabric/adapter/tests)
  add_subdirectory(stub/adapter/tests)
  add_subdirectory(ethereum/adapter/tests)
endif()
