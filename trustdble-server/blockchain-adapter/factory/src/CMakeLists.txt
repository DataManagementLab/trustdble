# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${TrustdbleadapterFactory_SOURCE_DIR}/include/stub_adapter/*.h")
set(HEADER_LIST 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_factory/adapter_factory.h"
  )

# Make an automatic library - will be static or dynamic based on user setting
add_library(adapterFactory adapter_factory.cpp ${HEADER_LIST})
# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::adapterFactory ALIAS adapterFactory)

# We need this directory, and users of our library will need it too
target_include_directories(adapterFactory PUBLIC ../include)

# This depends on (header only) boost
target_link_libraries(adapterFactory PUBLIC TrustDBle::adapterStub)
target_link_libraries(adapterFactory PUBLIC TrustDBle::adapterFabric)
target_link_libraries(adapterFactory PUBLIC TrustDBle::adapterEthereum)

# All users of this library will need at least C++17
target_compile_features(adapterFactory PUBLIC cxx_std_17)

# Help IDEs to find header files easier
target_sources(adapterFactory INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")