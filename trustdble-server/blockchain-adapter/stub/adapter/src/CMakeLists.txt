# Note that headers are optional, and do not affect add_library, but they will not
# show up in IDEs unless they are listed in add_library.

# Optionally glob, but only for CMake 3.12 or later:
#file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${TrustdbleStubAdapter_SOURCE_DIR}/include/adapter_stub/*.h")
set(HEADER_LIST 
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_stub/adapter_stub.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_stub/config_stub.h"
  )

add_library(blockNumber blocknumber.cpp "${CMAKE_CURRENT_SOURCE_DIR}/../include/adapter_stub/blocknumber.h")
target_include_directories(blockNumber PUBLIC ../include/adapter_stub)

# Make an automatic library - will be static or dynamic based on user setting
add_library(adapterStub adapter_stub.cpp ${HEADER_LIST})

# Add an alias so that library can be used inside the build tree, e.g. when testing
add_library(TrustDBle::adapterStub ALIAS adapterStub)

# We need this directory, and users of our library will need it too
target_include_directories(adapterStub PUBLIC ../include)

#target_link_libraries(adapterStub PRIVATE Boost::boost) # Header only
target_link_libraries(adapterStub PRIVATE ${Boost_LOG_LIBRARY} PRIVATE blockNumber) # Boost_LOG
target_link_libraries(adapterStub PUBLIC TrustDBle::adapterInterface)
target_link_libraries(adapterStub PUBLIC TrustDBle::adapterUtils)

# All users of this library will need at least C++17
target_compile_features(adapterStub PUBLIC cxx_std_17)
# The following uses generator expressions to add warnings based on plattform
target_compile_options(adapterStub PRIVATE $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall -Wextra -Wformat-security -Wvla -Wundef -Werror> $<$<CXX_COMPILER_ID:MSVC>: /W4>)

# Help IDEs to find header files easier
target_sources(adapterStub INTERFACE "$<BUILD_INTERFACE:${HEADERLIST}>")