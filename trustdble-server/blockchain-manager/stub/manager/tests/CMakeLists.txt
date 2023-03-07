include(test_macros)

package_add_test_with_libraries(manager_stub_test "${CMAKE_CURRENT_SOURCE_DIR}/stub-t.cpp" managerStub "${PROJECT_DIR}")
# this copies the files and replaces the CMake Variables that were used in the files with the actual values
configure_file(./test-config.ini ./ )

target_sources(manager_stub_test PRIVATE "${PROJECT_SOURCE_DIR}/interface/tests/bc_manager_interface_test.cpp")
target_include_directories(manager_stub_test PRIVATE "${PROJECT_SOURCE_DIR}/interface/tests")
