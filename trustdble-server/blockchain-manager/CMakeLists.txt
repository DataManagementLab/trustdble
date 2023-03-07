# Works with 3.16 and tested through 3.20
cmake_minimum_required(VERSION 3.16...3.20)

# Project name and a few useful settings. Other commands can pick up the results
project(
  BlockchainManager
  VERSION 0.1
  DESCRIPTION "Blockchain Manager project"
  LANGUAGES CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

include(FetchContent)
include(trustdble_configure)

add_subdirectory(interface)
add_subdirectory(stub/manager)
add_subdirectory(ethereum/manager)
add_subdirectory(fabric/manager)
add_subdirectory(factory)
add_subdirectory(cli)
add_subdirectory(utils)

#### Testing #####
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(CTest)
  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.11.0
  )
  set(CMAKE_POLICY_DEFAULT_CMP0048 NEW) # Required since googleTest uses an older cmake version
  FetchContent_MakeAvailable(googletest)
endif()

# Testing only available if this is the main app
# Emergency override MyProjectName_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR BlockchainManager_BUILD_TESTING)
   AND BUILD_TESTING)
  include(GoogleTest)
  add_subdirectory(ethereum/manager/tests)
  add_subdirectory(fabric/manager/tests)
  add_subdirectory(stub/manager/tests)
endif()
