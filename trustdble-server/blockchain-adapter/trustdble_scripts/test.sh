#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
# defined in .constants.sh
COMPILER=$DEFAULT_COMPILER
# Default target passed to cmake --build
ADAPTER="Adapter"
SANITIZER=""
RESET=false

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This runs tests for trustDBle server.
   Make sure to run './trustdble build --unit-test' before executing the tests.

   Usage: $PROGNAME [-a|--adapter]

   optional arguments:
     -h, --help                 show this help message and exit
     -a, --adapter              adapter for running (default: all) [Stub, Ethereum]

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
    -a|--adapter)
    ADAPTER=$2
    shift # past argument
    shift # past value
    ;;
    *) shift ;;     # not used parameters
esac
done

# Run tests with ctest
cd $BUILD_DIR_PREFIX$BUILD_TYPE
ctest -R ${ADAPTER} --output-on-failure
result=$?