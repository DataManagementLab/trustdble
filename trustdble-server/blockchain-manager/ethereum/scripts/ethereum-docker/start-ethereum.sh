#!/bin/bash

# variables
shardId=123
ethData="eth-data"
ethbasePwd=""
ethbasePrivateKey="resources/privateKey"
rpcPort=8000
minerThreads=1
difficulty=1

# Create private key if it not exists
bash generateKey.sh

# Create genesis file
echo "create genesisFile"
chainId=$((10+$shardId))
cat > genesis.json << EOL
{
  "config": {
        "chainId": ${chainId},
        "homesteadBlock": 0,
        "eip150Block": 0,
        "eip155Block": 0,
        "eip158Block": 0,
        "byzantiumBlock": 0,
        "constantinopleBlock": 0,
        "petersburgBlock": 0,
        "istanbulBlock": 0
  },
  "nonce": "0x0000000000000042",
  "timestamp": "0x0",
  "parentHash": "0x0000000000000000000000000000000000000000000000000000000000000000",
  "extraData": "0x",
  "gasLimit": "0x8000000",
  "difficulty": "$difficulty",
  "mixhash": "0x0000000000000000000000000000000000000000000000000000000000000000",
  "coinbase": "0x3333333333333333333333333333333333333333",
  "alloc": {
  }
}
EOL

echo "init geth and create ethbase"
echo "geth --datadir=$ethData init genesis.json"
geth --nousb --datadir=$ethData init genesis.json
# this only works in bash, not in sh/dash, see below for alternative
geth --nousb account import --datadir $ethData --password <(echo -n "$ethbasePwd") $ethbasePrivateKey

# alternative: create a pwd file
#echo -n "$ethbasePwd" >> /root/pwd
#geth account import --datadir $ethData --password /root/pwd /$(basename $ethbasePrivateKey)
# use this to create a new account instead of importing one
#geth --datadir=$ethData --password <(echo -n "$ethbasePwd") account new

echo "start mining"
# this only works in bash, not in sh/dash
geth --nousb --allow-insecure-unlock --datadir=$ethData --nodiscover --http --http.addr 0.0.0.0 --http.port "$rpcPort" --http.corsdomain "*" --miner.gasprice 0 --maxpeers 32 --networkid 9119 --unlock 0 --password <(echo -n "$ethbasePwd") --mine --miner.threads $minerThreads &

# alternative: create a pwd file
#echo "create tmp pwd file"
#echo -n "$ethbasePwd" >> /root/pwd
#geth --nousb --datadir=$ethData --nodiscover --rpc --rpcaddr 0.0.0.0 --rpcport "$rpcPort" --rpccorsdomain "*" --gasprice 0 --maxpeers 32 --networkid 9119 --unlock 0 --password /root/pwd --mine --minerthreads $minerThreads

bash