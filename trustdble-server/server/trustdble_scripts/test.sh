#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
TARGET=test-trustdble

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This runs tests for trustDBle server.
   Make sure to run './trustdble build --unit-test' before executing the tests.

   Usage: $PROGNAME [-t|--target]

   optional arguments:
     -h, --help                 show this help message and exit
     -t, --target               target for running (default: test-unit)
     -s, --sql                  sql test with mysql test framework 

HEREDOC
}

# sqltest FUNCTION
function sqltest()
{
  export DEPENDENCY_DIR=$(pwd)/build-release/_deps
  cd mysql-test/
  # write my.cnf only if not exists
  if [ ! -e suite/trustdble/my.cnf ]
    then
    tee  suite/trustdble/my.cnf <<EOF
[mysqld.1]
key_buffer_size=16M
max_allowed_packet=128M
binlog-format=STATEMENT

# Blockchain Storage eninge
blockchain_bc_configuration_path="$(pwd)/suite/trustdble/temp/configuration.ini"

rewrite_trustdble_bc_configuration_path="$(pwd)/suite/trustdble/temp/configuration.ini"
rewrite_trustdble_bc_dependency_path="${DEPENDENCY_DIR}"


rewrite_trustdble_server_number="0"

plugin-load=ha_blockchain.so;rewrite_trustdble.so

EOF
      
  fi

#create folder stub_data for data path if not exists
  if [ ! -e suite/trustdble/stub_data ]
    then
    mkdir -p suite/trustdble/stub_data
  fi

# create folder configs and write configuration.ini if not exists
 if [ ! -e suite/trustdble/temp ]
    then
    mkdir -p suite/trustdble/temp
  fi

# write configuration.ini only if not exists
  if [ ! -e suite/trustdble/temp/configuration.ini ]
    then
    tee  suite/trustdble/temp/configuration.ini <<EOF
[Adapter-Ethereum]
connection-url=http://127.0.0.1:8001
account=0x542BB0f7035f4bf7dB677a54B73e5d0514B9bfBC
max-waiting-time=300
script-path=${DEPENDENCY_DIR}/trustdble-adapters-src/ethereum/scripts
contract-path=${DEPENDENCY_DIR}/trustdble-adapters-src/ethereum/contract/truffle/build/contracts/SimpleStorage.json

[Adapter-Stub]
data-path=$(pwd)/suite/trustdble/stub_data/
blocksize=10

[Manager-Ethereum]
rpc-port=8000
peer-port=30303
join-ip=172.17.0.1
start-script-path=${DEPENDENCY_DIR}/blockchain-manager-src/ethereum/scripts/start-network.sh
docker-file-path=${DEPENDENCY_DIR}/blockchain-manager-src/ethereum/scripts/ethereum-docker

[Manager-Stub]
stub-network-path=$(pwd)/suite/trustdble/stub_data/

EOF
  fi
#./mysql-test-run.pl --nowarnings --force --suite=trustdble 
#./mysql-test-run.pl --nowarnings --suite=trustdble --force --record
./mysql-test-run.pl --nowarnings --do-test=STUB --force 
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
    -t|--target)
    TARGET=$2
    shift # past argument
    shift # past value
    ;;
    -s|--sql)
    sqltest
    exit 0
    ;;
    *) shift ;;     # not used parameters
esac
done

# Run tests
cmake --build $BUILD_DIR_PREFIX$BUILD_TYPE --target $TARGET

