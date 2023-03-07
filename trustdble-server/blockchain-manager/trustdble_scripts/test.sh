#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
# Default target passed to cmake --build
MANAGER="Manager"

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This runs tests for BCmanager.

   Usage: $PROGNAME [-a|--manager]

   optional arguments:
     -h, --help                 show this help message and exit
     -a, --manager              manager for running (default: all) [Ethereum, Fabric]

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
    -a|--manager)
    MANAGER=$2
    shift # past argument
    shift # past value
    ;;
    *) shift ;;     # not used parameters
esac
done

# Run tests with ctest
cd $BUILD_DIR_PREFIX$BUILD_TYPE
ctest -R ${MANAGER} --output-on-failure
result=$?
