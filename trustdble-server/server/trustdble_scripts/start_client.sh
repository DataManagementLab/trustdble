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
   This script can be used to start the trustDBle client with the passed instance number.

   Usage: $PROGNAME --number NUMBER

   required arguments:
     -n, --number NUMBER        The instance number of the server to which to connect this is used to distinguish between different servers

   optional arguments:
     -d, --docker               client connects to trusdble server running in docker container ( password='password')
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
    BUILD_TYPE=release
    shift # past argument
    ;;
esac
done

# LOGIC
if [ -z "$NUMBER" ]
  then
    echo "No instance number supplied, but required"
    exit 1
fi

re='^[0-9]+$'
if ! [[ $NUMBER =~ $re ]]
  then
    echo "[error] provided instance number is not valid!"
    exit 1
fi

if [ "$DOCKER" = "true" ];
  then
  PORT=$((${SERVER_PORT_BASE}+($NUMBER)))
  ${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysql --user root -p -P ${PORT} -h ${MYSQL_HOST}
else
  SERVER_HOME=$(pwd)/${SERVER_FOLDER_PREFIX}${NUMBER}/
  # set the config file
  MYSQL_HOME=${SERVER_HOME} ${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysql -h ${MYSQL_HOST}
fi