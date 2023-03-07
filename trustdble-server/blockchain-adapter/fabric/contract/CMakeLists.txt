# Compile contract
add_custom_target(fabricContract ALL
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ./gradlew clean && ./gradlew installDist
    )