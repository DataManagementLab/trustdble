#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
NUMBER=

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to remove a TrustDBle server with its data directory.
   This action is permanent and can not be undone.

   Usage: $PROGNAME  --number NUMBER

   required arguments:
     -n, --number NUMBER    The instance number of the server, this is used to distinguis between different servers

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
    -n|--number)
    NUMBER="$2"
    shift # past argument
    shift # past value
    ;;
    -d|--docker)
    DOCKER="true"
    shift # past argument
    ;;
    *) shift ;;     # not used parameters
esac
done

# LOGIC
if [ -z "$NUMBER" ] ; then
    echo "No instance number supplied, but required"
    exit 1
fi

re='^[0-9]+$'
if ! [[ $NUMBER =~ $re ]] ; then
    echo "[error] provided instance number is not valid!"
    exit 1
fi

SERVER_HOME_FOLDER=${SERVER_FOLDER_PREFIX}${NUMBER}

if ! [ -d "$SERVER_HOME_FOLDER" ] ; then
  echo "The server home directory '${SERVER_HOME_FOLDER}' does not exist."
else 
  echo "Removing server home directory '${SERVER_HOME_FOLDER}'."
  rm -rf ${SERVER_HOME_FOLDER}
fi

SERVER_DATA_FOLDER=${SERVER_HOME_FOLDER}${SERVER_DATA_FOLDER_SUFFIX}

if ! [ -d "$SERVER_DATA_FOLDER" ] ; then
  echo "The server data directory '${SERVER_DATA_FOLDER}' does not exist."
else 
  echo "Removing server data directory '${SERVER_DATA_FOLDER}'."
  rm -rf ${SERVER_DATA_FOLDER}
fi

if [ "$DOCKER" = "true" ]; then
  echo "Stopping and removing containers"
  docker container stop "trustdble-server-${NUMBER}"
  docker container rm "trustdble-server-${NUMBER}"
fi

# Remove any folders related to the databases on the server
rm -rf stub_data/data${NUMBER}*
rm -rf stub_data/meta${NUMBER}*

# stop docker container from this server if exists and remove it
docker container rm -f -v $(docker ps -a| grep "ethereum-node_meta${NUMBER}\|ethereum-node_data${NUMBER}" | awk '{print $1}' )
