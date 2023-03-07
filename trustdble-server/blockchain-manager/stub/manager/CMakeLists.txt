#find_package(Boost REQUIRED) # when header only required
FIND_PACKAGE(Boost REQUIRED COMPONENTS log)

# The compiled library code is here
add_subdirectory(src)