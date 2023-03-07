
# Since WITH_UNIT_TESTS is usually off (we do not want to execute all MySQL tests),
# we have to explicitly enable testing
IF (NOT WITH_UNIT_TESTS)
    ENABLE_TESTING()
    #include(googletest)
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
  test-rewrite-trustdble
  COMMAND ctest --progress --parallel ${CORES} --output-on-failure --timeout=300
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
)

## Add rewrite_trusdble test#################
# we need this config-file for the test
# Note: Currently, this is more of an integration test!
#configure_file(./test-config.ini ./ COPYONLY)
MYSQL_ADD_EXECUTABLE(rewrite_trustdble-t
    ${CMAKE_CURRENT_SOURCE_DIR}/rewrite_trustdble-t.cc
    ${CMAKE_CURRENT_SOURCE_DIR}/trustdble_invite-t.cc
    ADD_TEST rewrite_trustdble-t
)
SET_TARGET_PROPERTIES(rewrite_trustdble-t PROPERTIES ENABLE_EXPORTS TRUE)
TARGET_LINK_LIBRARIES(rewrite_trustdble-t rewriteTrustDBle)
TARGET_LINK_LIBRARIES(rewrite_trustdble-t trustDBleInvite)
TARGET_LINK_LIBRARIES(rewrite_trustdble-t gtest gmock gtest_main)
##########################################################