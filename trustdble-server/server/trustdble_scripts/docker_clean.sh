#!/bin/bash

# DEFAULTS

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to clean all trustdble specific docker container.

   optional arguments:
     -h, --help                 show this help message and exit
HEREDOC
}

# Stop and delete all docker containers where name starts with the given parameters 
function clean_containers()
{
  CONTAINERS=$(docker ps --filter name=$1* -aq)
  if [[ -z $CONTAINERS ]]; then
    echo "No $1 containers to clean up"
  else
    echo $CONTAINERS | xargs docker stop | xargs docker rm
  fi
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

echo "Stop and delete docker containers where name starts with ethereum-node"
clean_containers ethereum-node
echo 
echo "Stop and delete docker containers where name starts with trustdble-server"
clean_containers trustdble-server
echo
