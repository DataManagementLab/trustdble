
# Since WITH_UNIT_TESTS is usually off (we do not want to execute all MySQL tests),
# we have to explicitly enable testing
IF (NOT WITH_UNIT_TESTS)
    ENABLE_TESTING()
    include(googletest)
ENDIF()

# Run tests in parallel
INCLUDE(ProcessorCount)
ProcessorCount(CORES)
IF (CORES EQUAL 0)
  SET(CORES 1)
ENDIF()

# Since we enable testing in a subdirectory and not in the top-level CMakeLists.txt,
# we need to execute ctest in this subdirectory, see https://stackoverflow.com/questions/54550939/ctest-add-tests-in-subdirectories#comment95904971_54550939
ADD_CUSTOM_TARGET(
  test-trustdble
  COMMAND ctest  --parallel ${CORES} --output-on-failure --timeout=300
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)


INCLUDE_DIRECTORIES(
  ${CMAKE_SOURCE_DIR}/storage/blockchain/include
)

## Add stub adpater integration test#################
# we need this config-file for the test
# Note: Currently, this is more of an integration test!
configure_file(./test-config.ini ./ COPYONLY)
MYSQL_ADD_EXECUTABLE(stub-t
    stub-t.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/stub-t.cc
    ADD_TEST stub-t
)

MYSQL_ADD_EXECUTABLE(crypt_service-t
    crypt_service-t.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/crypt_service-t.cc
    ADD_TEST crypt_service-t
)
SET_TARGET_PROPERTIES(stub-t PROPERTIES ENABLE_EXPORTS TRUE)
TARGET_LINK_LIBRARIES(stub-t TrustDBle::adapterFactory)
TARGET_LINK_LIBRARIES(stub-t gtest gmock gtest_main)

SET_TARGET_PROPERTIES(crypt_service-t PROPERTIES ENABLE_EXPORTS TRUE)
TARGET_LINK_LIBRARIES(crypt_service-t gtest gmock gtest_main)
TARGET_LINK_LIBRARIES(crypt_service-t trustdbleCryptoService)
##########################################################