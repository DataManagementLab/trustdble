CMAKE_MINIMUM_REQUIRED(VERSION 3.14)

# Disable specific types of warnings for current directory
# if the compiler supports the flag
FOREACH(warning
    # Downgrade -Werror=suggest-override to warning
    "error=suggest-override"
    )
  MY_CHECK_CXX_COMPILER_WARNING("${warning}" HAS_WARN_FLAG)
  IF(HAS_WARN_FLAG)
    STRING_APPEND(CMAKE_CXX_FLAGS " ${HAS_WARN_FLAG}")
    STRING_APPEND(CMAKE_C_FLAGS " ${HAS_WARN_FLAG}")
  ENDIF()
ENDFOREACH()

add_subdirectory(../../../blockchain-adapter ./adapter)

# Make an automatic library - will be static or dynamic based on user setting
add_library(trustdbleCryptoService src/crypt_service.cc)
target_link_libraries(trustdbleCryptoService PUBLIC TrustDBle::trustdbleUtils TrustDBle::adapterFactory)

SET(BLOCKCHAIN_PLUGIN_DYNAMIC "ha_blockchain")
SET(BLOCKCHAIN_SOURCES
  src/ha_blockchain.cc
  src/table_service.cc
  src/transaction.cc
)
ADD_DEFINITIONS(-DMYSQL_SERVER)

# C++ 17
set(CMAKE_CXX_STANDARD 17)



# Blockchain storage engine includes
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(SYSTEM ${BOOST_PATCHES_DIR} ${BOOST_INCLUDE_DIR})

IF(WITH_BLOCKCHAIN_STORAGE_ENGINE AND NOT WITHOUT_BLOCKCHAIN_STORAGE_ENGINE)
  MYSQL_ADD_PLUGIN(blockchain ${BLOCKCHAIN_SOURCES} STORAGE_ENGINE DEFAULT)
ELSEIF(NOT WITHOUT_BLOCKCHAIN_STORAGE_ENGINE)
  MYSQL_ADD_PLUGIN(blockchain ${BLOCKCHAIN_SOURCES}
   STORAGE_ENGINE
   MODULE_ONLY
   LINK_LIBRARIES TrustDBle::adapterFactory trustdbleCryptoService
   )
ENDIF()

# Testing
OPTION(WITH_TRUSTDBLE_TESTS
  "Include the TRUSTDBLE test programs" OFF)
IF(WITH_TRUSTDBLE_TESTS)
  add_subdirectory(tests)
ELSE()
ENDIF()