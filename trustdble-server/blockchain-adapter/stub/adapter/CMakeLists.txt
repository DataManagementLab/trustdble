# find_package(Boost REQUIRED) # for header only dependencies
find_package(Boost REQUIRED COMPONENTS log)

# The compiled library code is here
add_subdirectory(src)