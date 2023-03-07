#!/bin/bash

# File containing constant values used across trustdble scripts.

# IMPORTANT:
# Constants are imported in trustdble.sh in the root directory.
# Make sure to use the 'export' statement for any variable to make it available for trustdble_scripts.

# TRUSTDBLE SERVER
export SERVER_FOLDER_PREFIX="mysql_home"
export SERVER_DATA_FOLDER_SUFFIX="data"
export SERVER_CONFIG_FILE_NAME="my.cnf"
export SERVER_PORT_BASE=3305

# TRUSTDBLE SERVER PLUGINS
export DEFAULT_PLUGINS="ha_blockchain.so;rewrite_trustdble.so"
export PLUGIN_FOLDER=build-debug/lib/plugin/
export PLUGIN_OUTPUT_FOLDER=build-debug/plugin_output_directory

# STUB BACKEND
export STUB_FOLDER_PREFIX="stub_data"

# METADATA
export META_FOLDER_PREFIX="meta_data"

# C++
export DEFAULT_CXX_COMPILER="c++"
export DEFAULT_C_COMPILER="cc"

# Clang/clang-tidy/format
export LLVM_VERSION=12

# Build folder
export BUILD_DIR_PREFIX=build-
if [[ -z "${BUILD_TYPE}" ]]; then
  echo "BUILD_TYPE not set, using debug"
  export BUILD_TYPE=debug
else
  echo "Using the following build type: ${BUILD_TYPE}"
fi

# Dependency folder
export DEPENDENCY_DIR=$(pwd)/..
