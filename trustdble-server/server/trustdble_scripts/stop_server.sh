#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
NUMBER=
DOCKER="false"
if [[ -z "${MYSQL_HOST}" ]]; then
  MYSQL_HOST="127.0.0.1"
  echo "MYSQL_HOST not set, using default value: ${MYSQL_HOST}"
else
  echo "Using the following MYSQL_HOST: ${MYSQL_HOST}"
fi

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to stop a TrustDBle server.

   Usage: $PROGNAME  --number NUMBER

   required arguments:
     -n, --number NUMBER    The instance number of the server, this is used to distinguis between different servers

   optional arguments:
     -h, --help                 show this help message and exit
     -d, --docker               for server runs as a docker container (default: false)

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

if [ "$DOCKER" = "true" ]; then
  echo "Stopping container"
  docker container stop "trustdble-server-${NUMBER}"
else
  SERVER_HOME=$(pwd)/${SERVER_FOLDER_PREFIX}${NUMBER}/
  # set the config file
  MYSQL_HOME=${SERVER_HOME} ${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysqladmin shutdown -h ${MYSQL_HOST}
fi
