 # Required for boost property tree
find_package(Boost REQUIRED)

# Header only library, therefore INTERFACE
add_library(bcManagerInterface INTERFACE)
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::bcManagerInterface ALIAS bcManagerInterface)
# INTERFACE targets only have INTERFACE propertis
target_include_directories(bcManagerInterface INTERFACE include/)
# Boost required for e.g. property tree
target_link_libraries(bcManagerInterface INTERFACE Boost::boost)

# Help IDEs find header files easier
set(HEADER_LIST 
    "${BcManagerInterface_SOURCE_DIR}/include/bc_manager_interface/bc_manager_interface.h"
    "${BcManagerInterface_SOURCE_DIR}/include/bc_manager_interface/bc_manager_config.h"
    )
target_sources(bcManagerInterface INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")