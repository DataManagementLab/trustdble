# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${TrustdblemanagerFactory_SOURCE_DIR}/include/stub_manager/*.h")
set(HEADER_LIST 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/bc_manager_factory/bc_manager_factory.h"
  )

# Make an automatic library - will be static or dynamic based on user setting
add_library(bcManagerFactory bc_manager_factory.cpp ${HEADER_LIST})
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::bcManagerFactory ALIAS bcManagerFactory)

# We need this directory, and users of our library will need it too
target_include_directories(bcManagerFactory PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(bcManagerFactory PUBLIC TrustDBle::managerEthereum)
target_link_libraries(bcManagerFactory PUBLIC TrustDBle::managerFabric)
target_link_libraries(bcManagerFactory PUBLIC TrustDBle::managerStub)

# All users of this library will need at least C++17
target_compile_features(bcManagerFactory PUBLIC cxx_std_17)

# Help IDEs to find header files easier
target_sources(bcManagerFactory INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")