#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script kills all currently running trustDBle server instances.

   Usage: $PROGNAME

   optional arguments:
     -h, --help                 show this help message and exit

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
    *) shift ;;     # not used parameters
esac
done

# LOGIC
sudo killall mysqld
