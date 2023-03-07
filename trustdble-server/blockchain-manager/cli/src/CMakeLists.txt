add_executable(managerCli manager_cli.cpp)
target_compile_features(managerCli PRIVATE cxx_std_17)

# this copies the files and replaces the CMake Variables that were used in the files with the actual values
configure_file(./config.ini ./ )

target_link_libraries(managerCli PRIVATE bcManagerFactory)