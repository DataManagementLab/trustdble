#!/bin/bash

containerName=${1:-ethereum-node}
rpcPort=${2:-8000}
peerPort=${3:-30303}
dockerFilePath=${4:-~/Projects/dm/blockchain-manager/ethereum/scripts/ethereum-docker}

# Build Docker image for ethereum node
cd $dockerFilePath
docker build . -t ethereum-tdb

# Start container with ethereum node image and expose ports and mount volume for eth-resources
docker run --name $containerName -v trustdble_eth-resources:/root/resources -dtp $rpcPort:8000 -p $peerPort:30303 ethereum-tdb