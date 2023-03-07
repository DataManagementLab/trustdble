#find_package(Boost REQUIRED) # when header only required
FIND_PACKAGE(Boost REQUIRED COMPONENTS log)
# curl
list(APPEND CMAKE_MODULE_PATH "/usr/lib/x86_64-linux-gnu/")
FIND_PACKAGE(CURL)

# The compiled library code is here
add_subdirectory(src)