#find_package(Boost REQUIRED) # when header only required
FIND_PACKAGE(Boost REQUIRED COMPONENTS log)

# The compiled library code is here
add_subdirectory(extern/go_client)
add_subdirectory(src)

# Compile contract
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../contract ${CMAKE_CURRENT_BINARY_DIR}/../contract)
