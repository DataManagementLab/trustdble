# Minimum CMake required
cmake_minimum_required(VERSION 3.15)

# Project
project(datacontract)

# gRPC
include(FetchContent)
FetchContent_Declare(
  gRPC
  GIT_REPOSITORY https://github.com/grpc/grpc
  GIT_TAG        v1.50.1
)
set(FETCHCONTENT_QUIET OFF)

# Protobuf-compiler
set(_PROTOBUF_PROTOC $<TARGET_FILE:protobuf::protoc>)

# Proto file
get_filename_component(dc_proto "datacontract.proto" ABSOLUTE)
get_filename_component(dc_proto_path "${dc_proto}" PATH)

# Generated sources
set(dc_grpc_src_py "${CMAKE_CURRENT_BINARY_DIR}/datacontract_pb2_grpc.py")
set(dc_proto_src_py "${CMAKE_CURRENT_BINARY_DIR}/datacontract_pb2.py")
set(dc_proto_src_pyi "${CMAKE_CURRENT_BINARY_DIR}/datacontract_pb2.pyi")
set(py_server_file_name "${dc_proto_path}/server.py")
set(py_server_file_name_des "${CMAKE_CURRENT_BINARY_DIR}/server.py")
set(py_read_logic_file_name "${dc_proto_path}/read_logic.py")
set(py_read_logic_file_name_des "${CMAKE_CURRENT_BINARY_DIR}/read_logic.py")
set(py_manifest_file_name "${dc_proto_path}/python.manifest.template")
set(py_manifest_file_name_des "${CMAKE_CURRENT_BINARY_DIR}/python.manifest.template")
set(py_makefile_file_name "${dc_proto_path}/Makefile")
set(py_makefile_file_name_des "${CMAKE_CURRENT_BINARY_DIR}/Makefile")
set(UDF "${dc_proto_path}/UDF.cc")
set(UDF_lib_des "${CMAKE_BINARY_DIR}/lib/plugin/UDF.so")
set(py_get_sgx_measurement "${dc_proto_path}/get_sgx_measurement.py")
set(py_get_sgx_measurement_des "${CMAKE_CURRENT_BINARY_DIR}/get_sgx_measurement.py")
set(ca_cert "${dc_proto_path}/ssl/trustdble_ca.crt")
set(ca_cert_dest "${CMAKE_CURRENT_BINARY_DIR}/trustdble_ca.crt")
set(ca_key "${dc_proto_path}/ssl/trustdble_ca.key")
set(ca_key_dest "${CMAKE_CURRENT_BINARY_DIR}/trustdble_ca.key")


find_package(PythonInterp REQUIRED)
find_package(Python3 REQUIRED)
add_custom_command(
      OUTPUT "${dc_grpc_src_py}" "${dc_proto_src_py}" "${dc_proto_src_pyi}"
      COMMAND python3 
      ARGS -m  grpc_tools.protoc
       -I "${dc_proto_path}"
      --python_out "${CMAKE_CURRENT_BINARY_DIR}"
      --pyi_out "${CMAKE_CURRENT_BINARY_DIR}"
      --grpc_python_out "${CMAKE_CURRENT_BINARY_DIR}"
      "${dc_proto}"
      DEPENDS "${dc_proto}")
add_custom_command(
  OUTPUT "${UDF_lib_des}"
    COMMAND g++ 
    ARGS -shared  -fpic -o "${UDF_lib_des}" "${UDF}"
    )
add_custom_target(run ALL DEPENDS "${dc_grpc_src_py}" "${UDF_lib_des}")

execute_process(COMMAND cp  ${py_server_file_name}  ${py_server_file_name_des})
execute_process(COMMAND chmod +x ${py_server_file_name_des})
execute_process(COMMAND cp  ${py_read_logic_file_name}  ${py_read_logic_file_name_des})
execute_process(COMMAND cp  ${py_manifest_file_name}  ${py_manifest_file_name_des})
execute_process(COMMAND cp  ${py_makefile_file_name}  ${py_makefile_file_name_des})
execute_process(COMMAND cp  ${py_get_sgx_measurement}  ${py_get_sgx_measurement_des})
execute_process(COMMAND cp  ${ca_cert}  ${ca_cert_dest})
execute_process(COMMAND cp  ${ca_key}  ${ca_key_dest})

# create an empty file DataContractLogic.py if already does not exist
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/DataContractLogic.py")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/DataContractLogic.py" "")
endif()
# create an empty file remote_server_cert.pem if already does not exist
if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/remote_server_cert.pem")
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/remote_server_cert.pem" "")
endif()
# Include generated *.pb.h files
include_directories("${CMAKE_CURRENT_BINARY_DIR}")
include_directories(${PROTOBUF_INCLUDE_DIRS})