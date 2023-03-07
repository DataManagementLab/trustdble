# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${TrustdbleEthereumAdapter_SOURCE_DIR}/include/manager_ethereum/*.h")
set(HEADER_LIST 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/manager_ethereum/manager_ethereum.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/manager_ethereum/config_ethereum.h"
  )

# Make an automatic library - will be static or dynamic based on user setting
add_library(managerEthereum manager_ethereum.cpp ${HEADER_LIST})
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::managerEthereum ALIAS managerEthereum)
# Dependency to go library
#add_dependencies(managerFabric curl)

# We need this directory, and users of our library will need it too
target_include_directories(managerEthereum PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(managerEthereum PRIVATE ${Boost_LOG_LIBRARY})
target_link_libraries(managerEthereum PUBLIC TrustDBle::bcManagerInterface)
target_link_libraries(managerEthereum PUBLIC TrustDBle::managerUtils )
target_link_libraries(managerEthereum PRIVATE ${CURL})

# All users of this library will need at least C++17
target_compile_features(managerEthereum PUBLIC cxx_std_17)

# Help IDEs to find header files easier
target_sources(managerEthereum INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")