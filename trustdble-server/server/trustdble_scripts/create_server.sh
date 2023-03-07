#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
PLUGINS=
STUB_NR=0
NUMBER=

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to create a config and install a trustDBle server with plugins.
   This server can then be started with the './trustdble start server' script in the next step.

   Usage: $PROGNAME  --number NUMBER [--plugins PLUGINS-STRING]

   required arguments:
     -n, --number NUMBER    The instance number of the server, this is used to distinguis between different servers

   optional arguments:
     -h, --help                 show this help message and exit
     -p, --plugins              semicolon seperated list (witout spaces) of plugins to install and use for this server instance (default: ha_blockchain.so,rewrite_trustdble.so)
     -s, --stub STUB_NUMBER     instance number of the stub backend that should be used for this server (default: '0')

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
    -s|--stub)
    STUB_NR="$2"
    shift # past argument
    shift # past value
    ;;
    -p|--plugins)
    if [ -z "$PLUGINS" ] ; then
      PLUGINS="$2"
    else
      PLUGINS="${PLUGINS},$2"
    fi

    shift # past argument
    shift # past value
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

if [ -z "$PLUGINS" ] ; then
  PLUGINS=${DEFAULT_PLUGINS}
fi

SERVER_HOME_FOLDER=${SERVER_FOLDER_PREFIX}${NUMBER}
SERVER_DATA_FOLDER=${SERVER_HOME_FOLDER}${SERVER_DATA_FOLDER_SUFFIX}
SERVER_CONFIG_FILE=${SERVER_HOME_FOLDER}/${SERVER_CONFIG_FILE_NAME}
SERVER_DEPENDENCY_CONFIG_FILE=$(pwd)/${SERVER_HOME_FOLDER}/configs/configuration.ini
PORT=$(($SERVER_PORT_BASE+$NUMBER))

if [ -d "$SERVER_HOME_FOLDER" ] ; then
  echo "The server home directory '${SERVER_HOME_FOLDER}' already exists."
  exit 1
elif [ -d "$SERVER_DATA_FOLDER" ] ; then
  echo "The server data directory '${SERVER_DATA_FOLDER}' already exists."
  exit 1
fi

# create config folder for server
mkdir -p ${SERVER_HOME_FOLDER}
mkdir -p ${SERVER_HOME_FOLDER}/configs
mkdir -p ${SERVER_HOME_FOLDER}/keys
mkdir -p ${SERVER_DATA_FOLDER}

# create template config file
user=$(whoami)
tee ${SERVER_CONFIG_FILE} <<EOF
[client]
user=root
#host=localhost
port=${PORT}
socket=/tmp/mysql${NUMBER}.sock

[mysqld]
user=$user
basedir=$(pwd)/${BUILD_DIR_PREFIX}${BUILD_TYPE}/
datadir=$(pwd)/${SERVER_DATA_FOLDER}/
port=${PORT}
socket=/tmp/mysql${NUMBER}.sock
key_buffer_size=16M
max_allowed_packet=128M
binlog-format=STATEMENT

EOF

# add blockchain storage engine specific configuration
tee --append ${SERVER_CONFIG_FILE} <<EOF
# Blockchain Storage eninge
blockchain_bc_configuration_path="${SERVER_DEPENDENCY_CONFIG_FILE}"


EOF

# add rewrite plugin specific configuration
tee --append ${SERVER_CONFIG_FILE} <<EOF
rewrite_trustdble_server_number="$NUMBER"
rewrite_trustdble_bc_configuration_path="${SERVER_DEPENDENCY_CONFIG_FILE}"
rewrite_trustdble_bc_dependency_path="${DEPENDENCY_DIR}"

EOF

# write adapter and manager configuration to file
tee ${SERVER_DEPENDENCY_CONFIG_FILE} <<EOF
[Adapter-Ethereum]
connection-url=http://127.0.0.1:8001
account=0x542BB0f7035f4bf7dB677a54B73e5d0514B9bfBC
max-waiting-time=300
script-path=${DEPENDENCY_DIR}/blockchain-adapter/ethereum/scripts
contract-path=${DEPENDENCY_DIR}/blockchain-adapter/ethereum/contract/truffle/build/contracts/SimpleStorage.json

[Adapter-Fabric]
adapters-path=${DEPENDENCY_DIR}/blockchain-adapter

[Adapter-Stub]
data-path=$(pwd)/${STUB_FOLDER_PREFIX}/
blocksize=10

[Manager-Ethereum]
rpc-port=8000
peer-port=30303
join-ip=172.17.0.1
start-script-path=${DEPENDENCY_DIR}/blockchain-manager/ethereum/scripts/start-network.sh
docker-file-path=${DEPENDENCY_DIR}/blockchain-manager/ethereum/scripts/ethereum-docker

[Manager-Fabric]
peer-port=7056
operations-port=9446
start-script-path=${DEPENDENCY_DIR}/blockchain-manager/fabric/scripts/network.sh
test-network-dir=~/TrustDBle/fabric-samples/test-network

[Manager-Stub]
stub-network-path=$(pwd)/${STUB_FOLDER_PREFIX}/

EOF

# install plugins and adjust config
PLUGINS="${PLUGINS//,/;}"
echo "${PLUGINS}"
install_plugins $PLUGINS

tee --append ${SERVER_CONFIG_FILE} <<EOF
plugin-load=${PLUGINS}

EOF

# initialize mysql server
echo "[Create TDB-Server script] Initializing mysql server"
${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysqld --datadir=$(pwd)/${SERVER_DATA_FOLDER} --basedir=$(pwd)/${SERVER_HOME_FOLDER} --initialize-insecure --user=$(whoami)

# Setup the rewrite_trustdble plugin
# First we have to start the server to be able to invoke the plugin's setup script

# set the home folder for all mysql invokations
export MYSQL_HOME=${SERVER_HOME_FOLDER}

# start mysql-server
echo "[Create TDB-Server script] Starting mysql server to install rewrite_trustdble plugin"
#MYSQL_HOME=${SERVER_HOME_FOLDER}
${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysqld --daemonize

# wait for server to start
for i in {30..0}; do
	if ${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysqladmin ping &>/dev/null; then
		break
	fi
	echo '[Create TDB-Server script] Waiting for server...'
	sleep 1
done
if [ "$i" = 0 ]; then
	echo >&2 '[Create TDB-Server script] Timeout during MySQL init.'
	exit 1
fi

# Generate a private key and public key pair
# Pair name
NAME=tdb-server-key
NAME_CERT=tdb-server-cert

# Key size
SIZE=4096

# Key pair destination.
DEST=${SERVER_HOME_FOLDER}/keys/

# Public/Private key files
PRIK=$DEST$NAME.pem
PUBK=$DEST$NAME.pub
CERT=$DEST$NAME_CERT.pem
CSR=$DEST$NAME_CERT.csr

# If either exists, avoid overwrite
if [ -f "$PRIK" ] || [ -f "$PUBK" ]; then
	echo "A key by that name already exists"
	exit 0
fi


# Generate password encrypted private key and plain public key

# This will prompt for a password which will be used to encrypt the
# private key
openssl genrsa -aes256 -passout pass:${NUMBER}${NAME} -out $PRIK $SIZE &&
openssl rsa -in $PRIK -passin pass:${NUMBER}${NAME} -pubout -out $PUBK

openssl req -key $PRIK -passin pass:${NUMBER}${NAME} -new -out $CSR -subj '/CN=localhost'
openssl x509 -req -in $CSR -CA ${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/data_contracts/trustdble_ca.crt -CAkey ${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/data_contracts/trustdble_ca.key -CAcreateserial -out $CERT -days 365000

# Setup meta data
for i in {2..0}; do
# Setup the plugin by calling its corresponding script to create meta data tables
	if ${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysql < ${DEPENDENCY_DIR}/src/plugin/rewrite_trustdble/scripts/setup_trustdble.sql  &>/dev/null; then
		break
	fi
	echo '[Create TDB-Server script] Creating meta data tables ...'
	sleep 1
done
if [ "$i" = 0 ]; then
	echo >&2 '[Create TDB-Server script] creating meta tables failed.'
  ${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysql -e "SHUTDOWN;"
	exit 1
fi

# Get SGX measuremnt
cwd=$(pwd)
cd ${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/data_contracts
# if the machine is sgx enabled resigned the manifest otherwise just insert default values in trustdble.enclave_measurement table
if grep -q -w "sgx" /proc/cpuinfo; then
  echo 're-sign the manifest and create files are used together to ensure the integrity and authenticity of the code running within the enclave...'
  make clean
  make SGX=1 RA_TYPE=dcap
  echo 'Insert enclave measurement in local table'
  python3 get_sgx_measurement.py ${PORT}
else
  echo 'Insert default values for enclave measurement in local table'
  python3 get_sgx_measurement.py ${PORT}
fi
cd ${cwd}

# Shutdown server after installation
echo "[Create TDB-Server script] Shutting down mysql server after plugin installation"
${BUILD_DIR_PREFIX}${BUILD_TYPE}/bin/mysql -e "SHUTDOWN;"

# copy needed files to create mysql udf into mysqldata folder
Src_Path="$(dirname "${BUILD_DIR_PREFIX}${BUILD_TYPE}")"
cp -r "${Src_Path}/plugin/data_contracts/client.py" "${SERVER_DATA_FOLDER}/client.py"
cp -r "${Src_Path}/plugin/data_contracts/read_logic.py" "${SERVER_DATA_FOLDER}/read_logic.py"
cp -r "${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/data_contracts/datacontract_pb2_data_contracts.py" "${SERVER_DATA_FOLDER}/datacontract_pb2_data_contracts.py"
cp -r "${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/data_contracts/datacontract_pb2.py" "${SERVER_DATA_FOLDER}/datacontract_pb2.py"
cp -r "${BUILD_DIR_PREFIX}${BUILD_TYPE}/plugin/data_contracts/datacontract_pb2.pyi" "${SERVER_DATA_FOLDER}/datacontract_pb2.pyi"

echo "Server ${NUMBER} was created successful."