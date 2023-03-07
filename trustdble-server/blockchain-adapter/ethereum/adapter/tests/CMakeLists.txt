include(test_macros)

package_add_test_with_libraries(adapter_ethereum_test "${CMAKE_CURRENT_SOURCE_DIR}/ethereum-t.cpp" adapterEthereum "${PROJECT_DIR}")
# this copies the files and replaces the CMake Variables that were used in the files with the actual values
configure_file(./test-config.ini ./ )

target_sources(adapter_ethereum_test PRIVATE "${PROJECT_SOURCE_DIR}/interface/tests/adapter_interface_test.cpp")
target_include_directories(adapter_ethereum_test PRIVATE "${PROJECT_SOURCE_DIR}/interface/tests")