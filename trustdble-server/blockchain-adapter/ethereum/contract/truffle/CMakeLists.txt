
# Compile solidity contract
add_custom_target(solidityContract ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND truffle compile
    )