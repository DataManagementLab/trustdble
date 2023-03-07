# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${TrustdbleEthereumAdapter_SOURCE_DIR}/include/adapter_ethereum/*.h")
set(HEADER_LIST 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_ethereum/adapter_ethereum.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_ethereum/config_ethereum.h"
  )

# Make an automatic library - will be static or dynamic based on user setting
add_library(adapterEthereum adapter_ethereum.cpp ${HEADER_LIST})
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::adapterEthereum ALIAS adapterEthereum)
# Dependency to go library
#add_dependencies(adapterFabric curl)

# We need this directory, and users of our library will need it too
target_include_directories(adapterEthereum PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(adapterEthereum PRIVATE ${Boost_LOG_LIBRARY})
target_link_libraries(adapterEthereum PUBLIC TrustDBle::adapterInterface)
target_link_libraries(adapterEthereum PRIVATE CURL::libcurl)
target_link_libraries(adapterEthereum PUBLIC TrustDBle::adapterUtils)

# All users of this library will need at least C++17
target_compile_features(adapterEthereum PUBLIC cxx_std_17)

# Help IDEs to find header files easier
target_sources(adapterEthereum INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")