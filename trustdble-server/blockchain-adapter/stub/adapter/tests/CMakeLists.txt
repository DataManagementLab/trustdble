include(test_macros)

package_add_test_with_libraries(adapter_stub_test "${CMAKE_CURRENT_SOURCE_DIR}/stub-t.cpp" adapterStub "${PROJECT_DIR}")
# Resources required for testing
configure_file(./test-config.ini ./ COPYONLY)

target_sources(adapter_stub_test PRIVATE "${PROJECT_SOURCE_DIR}/interface/tests/adapter_interface_test.cpp")
target_include_directories(adapter_stub_test PRIVATE "${PROJECT_SOURCE_DIR}/interface/tests")
