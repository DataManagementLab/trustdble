
set(GO_SRCS trustdbleClient.go)
set(GO_LIBNAME libclient.so)

# Run build_shared_go.sh to compile the go interface for fabric
add_custom_command(
    OUTPUT ${GO_LIBNAME} 
    DEPENDS ${GO_SRCS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND go build -buildmode c-shared -o "${CMAKE_CURRENT_BINARY_DIR}/${GO_LIBNAME}" ${GO_SRCS}
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/libclient.h" ./libclient.hpp
)
add_custom_target(libclient DEPENDS ${GO_LIBNAME})

IF (APPLE)
    add_custom_command(TARGET libclient
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/${GO_LIBNAME}" "${CMAKE_CURRENT_BINARY_DIR}/../../tests/${GO_LIBNAME}" 
    )
ENDIF()

# Copy libclient.so to binary directory
#configure_file(./libclient.h ${CMAKE_CURRENT_SOURCE_DIR}/libclient.hpp COPYONLY)
# target_link_libraries(adapterFabric PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/../extern/go_client/libclient.so")