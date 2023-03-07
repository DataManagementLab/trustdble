set(HEADER_LIST 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/manager_utils/shell_helpers.h"
  )

# Make an automatic library - will be static or dynamic based on user setting
add_library(managerUtils  shell_helpers.cpp ${HEADER_LIST})
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::managerUtils  ALIAS managerUtils )

# We need this directory, and users of our library will need it too
target_include_directories(managerUtils  PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(managerUtils  PRIVATE ${Boost_LOG_LIBRARY})
target_link_libraries(managerUtils  PRIVATE CURL::libcurl)

# All users of this library will need at least C++17
target_compile_features(managerUtils  PUBLIC cxx_std_17)

# Help IDEs to find header files easier
target_sources(managerUtils  INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")
