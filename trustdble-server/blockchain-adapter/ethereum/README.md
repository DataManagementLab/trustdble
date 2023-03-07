# TrustDBle Adapter for Ethereum

## Dependencies
- npm
- node
- truffle (npm install -g truffle)
- curl
  
## Getting Started
0. Search and replace, ie. adjust, all paths beginning with `/home/simon/Projects/dm`
1. Install truffle `sudo npm install -g truffle)
2. Compile the Ethereum smart contract `cd contract/truffle && truffle compile`

## Build
Build the code by running:
```
cmake -S . -B build-debug
cmake --build build-debug
``` 

or build with the help of the trustdble cmd:
```
./trustdble build
```

## Get Started with CLI
- Start an ethereum network e.g. by using the BlockchainManager
- Edit connection-url in ethereum/adapter/apps/config.ini if different from http://localhost:8000
- Rebuild:
```
cmake --build build-debug
```  
- Start cli: 
```
cd build-debug/ethereum/adapter/apps
./ethereumCli
```  

## Running the tests
1. Start an ethereum network e.g. by using the TrustDBle BlockchainManager
2. Build the code
3. Adjust the path to the ethereum scripts in the test-config.ini
4. Run the tests
```
cmake --build build-debug --target test -- ARGS="-R Ethereum* --output-on-failure --verbose"
```