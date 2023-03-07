add_executable(adapterCli adapter_cli.cpp)
target_compile_features(adapterCli PRIVATE cxx_std_17)

# this copies the files and replaces the CMake Variables that were used in the files with the actual values
configure_file(./config.ini ./ )

# make default stub_data folder in build fold
set(directory "${CMAKE_CURRENT_BINARY_DIR}/../stub_data")
file(MAKE_DIRECTORY ${directory})

target_link_libraries(adapterCli PRIVATE adapterFactory)