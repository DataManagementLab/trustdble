 # Required for boost property tree
find_package(Boost REQUIRED)

# Header only library, therefore INTERFACE
add_library(adapterInterface INTERFACE)
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::adapterInterface ALIAS adapterInterface)
# INTERFACE targets only have INTERFACE propertis
target_include_directories(adapterInterface INTERFACE include/)
# Boost required for e.g. property tree
target_link_libraries(adapterInterface INTERFACE Boost::boost)

# Help IDEs find header files easier
set(HEADER_LIST 
    "${TrustdbleInterfaceAdapter_SOURCE_DIR}/include/adapter_interface/adapter_interface.h"
    "${TrustdbleInterfaceAdapter_SOURCE_DIR}/include/adapter_interface/adapter_config.h"
    )
target_sources(adapterInterface INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")