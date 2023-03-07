# TrustDBle Adapter for Hyperledger Fabric

## Requirements
The following additional software is required to run a Hyperledger Fabric blockchain and use this TrustDBle adapter implementation to interact with it
- Docker
- Docker-Compose
- Java 

# Steps to get fabric running as an adapter for blockchain storage engine  

1. Create a fabric network using the blockchain-manager. The peers need to have at least version 2.4

2. When asked for the network config. Enter the config printed by the blockchain-manager

## Running the adapter tests

1. Create a fabric network using the blockchain-manager. The peers need to have at least version 2.4

2. Enter the network config in `fabric/adapter/tests/fabric-t.cpp`. Note that C++ uses different escape characters than the console output.

3. Build the code with the help of the trustdble cmd:
```
./trustdble build
```

4. Run the Fabric adapter tests (Note the fabric blockchain is started automatically)
```
./trustdble test -a Fabric
```