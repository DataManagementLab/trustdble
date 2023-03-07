#!/bin/bash

SCRIPT_PATH=$(cd "$(dirname "$0")" && pwd)
ADMIN_NAME="admin"
ADMIN_PASSWORD="adminpw"
CA_ADDRESS="localhost:7054"
CA_NAME="ca-org1"
export ORG_FULLNAME="org1.example.com"
export ORG_NAME="org1"
export LOCALMSPID="${ORG_NAME^}MSP"

# Get docker sock path from environment variable
SOCK="${DOCKER_HOST:-/var/run/docker.sock}"
export DOCKER_SOCK="${SOCK##unix://}"

function createCerts(){
  echo "Generating certificates using Fabric CA"
  export FABRIC_CA_CLIENT_HOME=${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/

  fabric-ca-client enroll -u "https://${ADMIN_NAME}:${ADMIN_PASSWORD}@${CA_ADDRESS}" --caname "${CA_NAME}" --tls.certfiles "${PWD}/organizations/fabric-ca/${ORG_NAME}/ca-cert.pem"
  if [ "$?" -ne 0 ]; then
    echo "Error enrolling admin user. This error might be caused by the test-network not starting the ca-servers, if the test-network wasn't reset. To reset it run ./network.sh down in your test-network directory."
    exit 1
  fi

  echo "Registering new peer"
  PEER_PASSWORD=$(cat /dev/urandom | tr -dc '[:alpha:]' | fold -w 20 | head -n 1)
  fabric-ca-client register --caname "${CA_NAME}" --id.name ${PEER_NAME} --id.secret ${PEER_PASSWORD} --id.type peer --tls.certfiles "${PWD}/organizations/fabric-ca/${ORG_NAME}/ca-cert.pem"
  if [ "$?" -ne 0 ]; then
    echo "error registering new peer"
    exit 1
  fi

  # error in echo below orgs have msps peers have certs
  echo "Generating the new peer's msp"
  fabric-ca-client enroll -u "https://${PEER_NAME}:${PEER_PASSWORD}@${CA_ADDRESS}" --caname "${CA_NAME}" -M "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/msp" --csr.hosts "${PEER_NAME}" --tls.certfiles "${PWD}/organizations/fabric-ca/${ORG_NAME}/ca-cert.pem"
  if [ "$?" -ne 0 ]; then
    echo "error generating the new peer's msp"
    exit 1
  fi

  cp "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/msp/config.yaml" "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/msp/config.yaml"

  echo "Generating the new peer's tls certificates"
  fabric-ca-client enroll -u "https://${PEER_NAME}:${PEER_PASSWORD}@${CA_ADDRESS}" --caname ${CA_NAME} -M "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/tls" --enrollment.profile tls --csr.hosts "${PEER_NAME}" --csr.hosts localhost --tls.certfiles "${PWD}/organizations/fabric-ca/${ORG_NAME}/ca-cert.pem"
  if [ "$?" -ne 0 ]; then
    echo "error generating the new peer's tls certificates"
    exit 1
  fi

  # Copy the tls CA cert, server cert, server keystore to well known file names in the peer's tls directory that are referenced by peer startup config
  cp "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/tls/tlscacerts/"* "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/tls/ca.crt"
  cp "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/tls/signcerts/"* "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/tls/server.crt"
  cp "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/tls/keystore/"* "${PWD}/organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}/tls/server.key"
}

function joinChannel(){
  echo "Joining new peer to channel"

  export FABRIC_CFG_PATH=$TESTNETWORK_PATH/../config/

  BLOCKFILE="${TESTNETWORK_PATH}/channel-artifacts/${NETWORK_NAME}.block"

  if [ ! -f "$BLOCKFILE" ]; then
    echo "the network you're trying to join doesn't exist or was deleted"
    return 1
  fi

  export CORE_PEER_LOCALMSPID="${LOCALMSPID}"
  export CORE_PEER_TLS_ROOTCERT_FILE=$TESTNETWORK_PATH/organizations/peerOrganizations/${ORG_FULLNAME}/tlsca/tlsca.${ORG_FULLNAME}-cert.pem
  export CORE_PEER_MSPCONFIGPATH=$TESTNETWORK_PATH/organizations/peerOrganizations/${ORG_FULLNAME}/users/${ADMIN_NAME^}@${ORG_FULLNAME}/msp
  export CORE_PEER_ADDRESS="localhost:${PEER_PORT}"
  export CORE_PEER_TLS_ENABLED=true

  local rc=1
	local COUNTER=1
  local MAX_RETRY=5
  local DELAY=3
  ## Sometimes Join takes time, hence retry
  while [ $rc -ne 0 -a $COUNTER -lt $MAX_RETRY ] ; do
    sleep $DELAY
    peer channel join -b $BLOCKFILE
    let rc=$?
    COUNTER=$(expr $COUNTER + 1)
  done
  if [ $rc -ne 0 ]; then
    echo "error joining new peer to channel"
    return 1
  fi
  return 0
}

function up(){
	echo ""
  echo "###################################################################"
  echo "# 	Starting fabric network"
  echo "##################################################################"
  echo ""

  if [ ! -d "$TESTNETWORK_PATH" ]; then
    echo "directory $TESTNETWORK_PATH not found"
    exit 1
  fi
	cd $TESTNETWORK_PATH
  # with -ca Certificate Authorities are used to create certificates
  # with -c the network name is used as channel name
	./network.sh up createChannel -ca -c $NETWORK_NAME
  if [ "$?" -ne 0 ]; then
    echo "error starting the fabric network"
    exit 1
  fi
	echo "Fabric network started!"
	
}

function down(){
  echo ""
  echo "###################################################################"
  echo "# 	Shutting down fabric network"
  echo "##################################################################"
  echo ""

  if [ ! -d "$TESTNETWORK_PATH" ]; then
    echo "directory $TESTNETWORK_PATH not found"
    exit 1
  fi
	cd $TESTNETWORK_PATH
	./network.sh down
  if [ "$?" -ne 0 ]; then
    echo "error shutting down the fabric network"
    exit 1
  fi
	echo "Fabric network shut down!"
}

function join(){
  echo ""
  echo "###################################################################"
  echo "# 	Joining new peer to fabric network"
  echo "##################################################################"
  echo ""

  if [ ! -d "$TESTNETWORK_PATH" ]; then
    echo "directory $TESTNETWORK_PATH not found"
    exit 1
  fi
  cd $TESTNETWORK_PATH
  # generate artifacts if they don't exist
  if [ ! -d "organizations/peerOrganizations/${ORG_FULLNAME}/peers/${PEER_NAME}" ]; then
    createCerts
  fi

  # this variable is prepended to the container's name. the name is set this way,
  # because docker-compose doesn't support variables in service names
  export COMPOSE_PROJECT_NAME="${PEER_NAME}"
  docker-compose -f ${SCRIPT_PATH}/fabric-docker/docker-compose-test-peer.yaml up -d 2>&1
  if [ "$?" -ne 0 ]; then
    echo "error starting the new peer"
    exit 1
  fi

  # join peer to channel

  joinChannel
  if [ "$?" -ne 0 ]; then
    echo "error after starting container- remove container"
    remove
    exit 1
  fi
  # do we need to deploy cc on peer?
}

function remove(){
  echo ""
  echo "###################################################################"
  echo "# 	Removing peer"
  echo "##################################################################"
  echo ""

  # export variables needed in docker-compose.yaml
  # their values don't have to match the values used when creating the container
  export COMPOSE_PROJECT_NAME="${PEER_NAME}"
  export TESTNETWORK_PATH=""
  export PEER_PORT=0
  export OPERATIONS_PORT=1
  export NETWORK_NAME=""
  docker-compose -f ${SCRIPT_PATH}/fabric-docker/docker-compose-test-peer.yaml down --volumes
  if [ "$?" -ne 0 ]; then
    echo "error removing the peer"
    exit 1
  fi
	echo "Peer removed!"
}

# Print the usage message
function printHelp() {
  echo "Usage: "
  echo "  network.sh COMMAND"
  echo
  echo "    commands:"
  echo "    up     NETWORK_DIRECTORY          - start the network"
  echo "      --network-name  NETWORK_NAME"
  echo
  echo "    down   NETWORK_DIRECTORY          - shutdown the network"
  echo
  echo "    join   NETWORK_DIRECTORY          - join a peer to the network"
  echo "      --network-name     NETWORK_NAME"
  echo "      --peer-name        PEER_NAME"
  echo "      --peer-port        PEER_PORT"
  echo "      --operations-port  OPERATIONS_PORT"
  echo
  echo "    remove                            - remove a peer from the network"
  echo "      --peer-name     PEER_NAME"
  echo
  echo "    h                                 - print this message"
}

###############
# Main script #
###############


# parse arguments

key="$1"
case $key in
up )
  TESTNETWORK_PATH=${2:-~/TrustDBle/fabric-samples/test-network}
  export PATH=${TESTNETWORK_PATH}/../bin:$PATH
  shift
  shift
  while [[ $# -ge 1 ]] ; do
    key="$1"
    case $key in
    --network-name )
      NETWORK_NAME="$2"
      shift
      ;;
    * )
      echo
      echo "Unknown flag: $key"
      echo
      printHelp
      exit 1
      ;;
    esac
    shift
  done
  up
  exit 0
  ;;
down )
  TESTNETWORK_PATH=${2:-~/TrustDBle/fabric-samples/test-network}
  export PATH=${TESTNETWORK_PATH}/../bin:$PATH
  down
  exit 0
  ;;
join )
  export TESTNETWORK_PATH=${2:-~/TrustDBle/fabric-samples/test-network}
  export PATH=${TESTNETWORK_PATH}/../bin:$PATH
  shift
  shift
  while [[ $# -ge 1 ]] ; do
    key="$1"
    case $key in
    --network-name )
      export NETWORK_NAME="$2"
      shift
      ;;
    --peer-name )
      export PEER_NAME="$2"
      shift
      ;;
    --peer-port )
      export PEER_PORT="$2"
      shift
      ;;
    --operations-port )
      export OPERATIONS_PORT="$2"
      shift
      ;;
    * )
      echo
      echo "Unknown flag: $key"
      echo
      printHelp
      exit 1
      ;;
    esac
    shift
  done
  join
  exit 0
  ;;
remove )
  TESTNETWORK_PATH=${2:-~/TrustDBle/fabric-samples/test-network}
  export PATH=${TESTNETWORK_PATH}/../bin:$PATH
  shift
  while [[ $# -ge 1 ]] ; do
    key="$1"
    case $key in
    --peer-name )
      export PEER_NAME="$2"
      shift
      ;;
    * )
      echo
      echo "Unknown flag: $key"
      echo
      printHelp
      exit 1
      ;;
    esac
    shift
  done
  remove
  exit 0
  ;;
h )
  printHelp
  exit 0
  ;;
* )
  echo
  echo "Unknown command: $key"
  echo
  printHelp
  exit 1
  ;;
esac

printHelp
exit 0

