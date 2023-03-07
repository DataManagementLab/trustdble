#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
# defined in .constants.sh
COMPILER=$DEFAULT_COMPILER
# Default target passed to cmake --build
TARGET="all"
SANITIZER=""
RESET=false

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script builds the blockchain manager project into the 'build' directory.

   Usage: $PROGNAME [--compiler COMPILER] [-t|--target] [-s|--sanitizer] [--release] [--reset]

   required arguments:

   optional arguments:
     -h, --help                 show this help message and exit
     -c, --compiler COMPILER    pass in the compiler to use, ie., either 'g++' or 'clang' (default: clang)
     -t, --target               pass in the cmake target to build (default: 'all', common targets 'help'|'docs')
     -s, --sanitizer            compile with sanitizer flags enabled, currently, AddessSanitizer and UndefinedBehavior. (default: false)
     --release                  set build type to release (default: Debug)
     --reset                    delete content of build dir, for fresh cmake generation. (default: false)

HEREDOC
}

# ARGUMENTS
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    usage
    exit 0
    ;;
    -c|--compiler)
    COMPILER="$2"
    shift # past argument
    shift # past value
    ;;
    -t|--target)
    TARGET="$2"
    shift # past argument
    shift # past value
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
    *) shift ;;     # not used parameters
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

echo "Building $BUILD_TYPE."

# -S refers to the folder with the source files and -B to the build folder
cmake -S . -B $BUILD_DIR_PREFIX$BUILD_TYPE -DCMAKE_CXX_COMPILER=$COMPILER -DCMAKE_BUILD_TYPE=$BUILD_TYPE -GNinja  $SANITIZER

cmake --build $BUILD_DIR_PREFIX$BUILD_TYPE --target $TARGET
