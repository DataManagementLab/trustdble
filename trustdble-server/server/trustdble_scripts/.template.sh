#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
EXTENSION=
RESET=NO

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script does ...

   Usage: $PROGNAME --extension EXTENSION [--reset]

   required arguments:
     -e, --extension EXTENSION    pass in the extension to use

   optional arguments:
     -h, --help                 show this help message and exit
     -r, --reset                delete content of build dir, for fresh cmake generation, default false (lowercase)

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
    -e|--extension) # parameter with value
    EXTENSION="$2"
    shift # past argument
    shift # past value
    ;;
    -r|--reset)      # parameter without value (flag)
    RESET=YES
    shift # past argument
    ;;
    *) shift ;;     # not used parameters
esac
done

# PARAMETER CHECK
if [ -z "$EXTENSION" ] ; then
    echo "No extension argument supplied, but required"
    exit 1
fi

# FUNCTIONS
function print_var { echo "$1: $2"; }

# LOGIC
print_var extension $EXTENSION
print_var reset $RESET