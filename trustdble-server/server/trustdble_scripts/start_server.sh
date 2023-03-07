#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
CLEAN=false
PLUGINS=
NUMBER=
if [[ -z "${MYSQL_ROOT_HOST}" ]]; then
  MYSQL_ROOT_HOST="$(ip -4 addr show docker0 | grep -oP '(?<=inet\s)\d+(\.\d+){3}')"
  echo "MYSQL_ROOT_HOST not set, using value of docker0: ${MYSQL_ROOT_HOST}"
else
  echo "Using the following MYSQL_ROOT_HOST: ${MYSQL_ROOT_HOST}"
fi
if [[ -z "${JOIN_IP}" ]]; then
  echo "JOIN_IP not set, using value of MYSQL_ROOT_HOST: ${MYSQL_ROOT_HOST}"
  JOIN_IP=${MYSQL_ROOT_HOST}
else
  echo "Using the following JOIN_IP: ${JOIN_IP}"
fi


# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to start the trustdble server with the passed instance number.
   Make sure to call the 'create server' script with the corresponding instance number first, if you are not running the server in a container (using the -d flag).

   Usage: $PROGNAME --number NUMBER

   required arguments:
     -n, --number NUMBER        The instance number of the server, this is used to distinguis between different servers

   optional arguments:
     -h, --help                 show this help message and exit
     -i, --ip                   ip of the local network or vpn to reach your host
     -d, --docker               Run the trustdble-server inside a docker container (does not require executing the create server script previously)
     -c, --clean                create a server also if there is an existing one.  The old one will be removed and a new one is created
     
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
    -i|--ip)
    JOIN_IP="$2"
    shift # past argument
    shift # past value
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
    -c|--clean)
    CLEAN=true
    shift # past value
    ;;
    *) shift ;;     # not used parameters
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


SERVER_HOME=$(pwd)/${SERVER_FOLDER_PREFIX}${NUMBER}


# clean a server: 
if [ ${CLEAN} == true ] && [ -d "$SERVER_HOME" ]; then
    echo 
    echo "Cleaning server ${NUMBER}"
  if [ "$DOCKER" = "true" ];  then
    bash trustdble_scripts/remove_server.sh -n $NUMBER -d
  else
    bash trustdble_scripts/remove_server.sh -n $NUMBER
  fi
fi

# check if there is an existing server if not it will automatically create one.
if ! [ -d "$SERVER_HOME" ] ; then
  echo 
  echo "Creating server ${NUMBER}"
  bash trustdble_scripts/create_server.sh -n $NUMBER
  retn_code=$?
  if [[ "$retn_code" -eq 1 ]]; then
    echo "failed to create server ${NUMBER}"
		exit 1
	fi
  echo 
fi

# run gRPC server to start listening 
cwd=$(pwd)
 if grep -q -w "sgx" /proc/cpuinfo; then 
  cd ${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/gRPC
  gramine-sgx ./python server.py 0 & >> errlog 2>&1
else
  cd ${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/gRPC
  python3 server.py 1 ${SERVER_HOME}/keys/tdb-server-key.pem ${NUMBER}tdb-server-key ${SERVER_HOME}/keys/tdb-server-cert.pem & >> errlog 2>&1
fi
cd ${cwd}
# Set environment variables
export STUB_PATH=$(pwd)/${STUB_FOLDER_PREFIX}
mkdir -p ${STUB_PATH};
if [ "$DOCKER" = "true" ]; then
  DOCKER_PORT=$(($SERVER_PORT_BASE+$NUMBER))
  echo "Start trustdble-server${NUMBER} on port ${DOCKER_PORT}"
  docker run --name trustdble-server-${NUMBER} -it -v /var/run/docker.sock:/var/run/docker.sock -v ${STUB_PATH}:/${STUB_FOLDER_PREFIX}  -e STUB_PATH=/$STUB_FOLDER_PREFIX -e MYSQL_ROOT_PASSWORD=password -e MYSQL_ROOT_HOST=${MYSQL_ROOT_HOST} -e ETH_JOIN_IP=${JOIN_IP} -p $DOCKER_PORT:3305 trustdble-server

#  echo "docker restart..."
  docker restart $(docker ps -a| grep "ethereum-node_data${NUMBER}" | awk '{print $1}' )
  docker restart $(docker ps -a| grep "ethereum-node_meta${NUMBER}" | awk '{print $1}' )

else
  SERVER_CONFIG_FILE=${SERVER_HOME}/${SERVER_CONFIG_FILE_NAME}

  load_plugins ${SERVER_CONFIG_FILE}
  echo "plugins loaded: $PLUGINS"
  install_plugins $PLUGINS

  export ETH_JOIN_IP=$JOIN_IP
  export MYSQL_ROOT_HOST=$MYSQL_ROOT_HOST
  
  # restart all stoped container of this server
  echo "docker restart..."
  docker restart $(docker ps -a| grep "ethereum-node_data${NUMBER}" | awk '{print $1}' )
  docker restart $(docker ps -a| grep "ethereum-node_meta${NUMBER}" | awk '{print $1}' )

  # set the config and start mysql-server
  MYSQL_HOME=${SERVER_HOME} ${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysqld --federated
fi
lsof -ti tcp:50051 | xargs kill