#find_package(Boost REQUIRED) # when header only required
FIND_PACKAGE(Boost REQUIRED COMPONENTS log)
# curl
list(APPEND CMAKE_MODULE_PATH "/usr/lib/x86_64-linux-gnu/")
FIND_PACKAGE(CURL)

# Compile solidity contract
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../contract/truffle ${CMAKE_CURRENT_BINARY_DIR}/../contract/truffle)

# Install npm modules for scripts
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../scripts ${CMAKE_CURRENT_BINARY_DIR}/../scripts)

# The compiled library code is here
add_subdirectory(src)
