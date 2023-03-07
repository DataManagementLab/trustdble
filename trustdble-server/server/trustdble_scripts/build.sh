#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
CXX_COMPILER=$DEFAULT_CXX_COMPILER
C_COMPILER=$DEFAULT_C_COMPILER
UNIT_TESTS=OFF
TRUSTDBLE_TESTS=OFF
SANITIZER=""
RESET=false

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script builds the trustDBle server project into the 'build' directory.

   Usage: $PROGNAME [-c|--c-compiler C_COMPILER] [-cxx|--cxx-compiler CXX_COMPILER] [-u|--unit-test] [-s|--sanitizer] [--release] [--reset]

   required arguments:

   optional arguments:
     -h, --help				show this help message and exit
     -cxx, --cxx-compiler CXX_COMPILER  pass in the c++ compiler to use, ie., either 'g++' or 'clang++' (default: ${CXX_COMPILER})
     -c, --c-compiler C_COMPILER  	pass in the c compiler to use, ie., either 'g' or 'clang' (default: ${C_COMPILER})
     -u, --unit-test            	build with (trustdble) unit tests (default: false)
     -a, --all-tests            	build with all unit tests (trustdble and mysql) (default: false)
     -s, --sanitizer            	compile with sanitizer flags enabled, currently, AddessSanitizer and UndefinedBehavior. (default: false)
     --release                  	set build type to release (default: Debug)
     --reset                    	delete content of build dir, for fresh cmake generation. (default: false)

HEREDOC
}

# ARGUMENTS
CMAKE_PARAMS=() # params that will be passed
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    usage
    exit 0
    ;;
    -cxx|--cxx-compiler)
    CXX_COMPILER="$2"
    shift # past argument
    shift # past value
    ;;
    -c|--c-compiler)
    C_COMPILER="$2"
    shift # past argument
    shift # past value
    ;;
    -u|--unit-test)
    TRUSTDBLE_TESTS=ON
    shift # past argument
    ;;
    -a| --all-tests)
    UNIT_TESTS=ON
    TRUSTDBLE_TESTS=ON
    shift # past argument
    ;;
    -s|--sanitizer)
    SANITIZER="-DWITH_ASAN=ON -DWITH_UBSAN=ON"
    shift # past argument
    ;;
    --release)
    BUILD_TYPE=release
    shift # past argument
    ;;
    --reset)
    RESET=true
    shift # past argument
    ;;
    *)
    CMAKE_PARAMS+=("$1") # save it in an array for later
    shift      # not used parameter
    ;;
esac
done

# ARGUMENT CHECK

# Enforce argument combinations
if [ $BUILD_TYPE == release ] ; then
  SANITIZER=""
  UNIT_TESTS=OFF
elif [ $BUILD_TYPE == debug ] && [ $UNIT_TESTS == OFF ] ; then
  SANITIZER=""
fi

if [ "${RESET}" = true ] ; then
    echo "Clearing build directory and start fresh"
    # remove old content of build folder
    rm -rf $BUILD_DIR_PREFIX$BUILD_TYPE/*
  else
    echo "Re-using content of build directory"
fi

echo "Passing the following script args to command ${CMAKE_PARAMS[@]}"

echo "Building $BUILD_TYPE $(if [ $UNIT_TESTS == ON ]; then echo with; else echo without; fi) unit tests."

# -S refers to the folder with the source files and -B to the build folder
cmake -S . -B $BUILD_DIR_PREFIX$BUILD_TYPE \
  -DCMAKE_C_COMPILER=$C_COMPILER \
  -DCMAKE_CXX_COMPILER=$CXX_COMPILER \
  -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
  -GNinja \
  -DWITH_UNIT_TESTS=$UNIT_TESTS \
  -DWITH_TRUSTDBLE_TESTS=$TRUSTDBLE_TESTS \
  -DENABLE_DOWNLOADS=1 \
  -DCMAKE_CXX_STANDARD_LIBRARIES="-lcurl -lssl -lcrypto" \
  $SANITIZER \
  ${CMAKE_PARAMS[@]}

cmake --build $BUILD_DIR_PREFIX$BUILD_TYPE --parallel
