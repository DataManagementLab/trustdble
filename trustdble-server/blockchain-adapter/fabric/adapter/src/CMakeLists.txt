# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${TrustdbleFabricAdapter_SOURCE_DIR}/include/adapter_fabric/*.h")
set(HEADER_LIST 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_fabric/adapter_fabric.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_fabric/client_fabric.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_fabric/config_fabric.h"
  )

# Make an automatic library - will be static or dynamic based on user setting
add_library(adapterFabric adapter_fabric.cpp client_fabric.cpp ${HEADER_LIST})
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::adapterFabric ALIAS adapterFabric)
# Dependency to go library
add_dependencies(adapterFabric libclient)

# We need this directory, and users of our library will need it too
target_include_directories(adapterFabric PUBLIC ../include)

# This depends on (header only) boost and nlohmann_json
target_link_libraries(adapterFabric PRIVATE ${Boost_LOG_LIBRARY})
target_link_libraries(adapterFabric PUBLIC TrustDBle::adapterInterface)
target_link_libraries(adapterFabric PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../extern/go_client/libclient.so)
target_link_libraries(adapterFabric PUBLIC TrustDBle::adapterUtils)

# All users of this library will need at least C++17
target_compile_features(adapterFabric PUBLIC cxx_std_17)

# Help IDEs to find header files easier
target_sources(adapterFabric INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")