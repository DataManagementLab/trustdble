
# Compile solidity contract
add_custom_target(npmModules ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND npm install
    )