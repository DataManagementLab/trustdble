# Blockchain Manager
The Blockchain Manager can be used to automatically deploy a blockchain network or join an existing one.
Currently Ethereum and Fabric are supported.

## Getting Started
**Note:** We provide a development container for Ethereum and Stub that automatically installs all required dependencies (its initial setup may take a while).

0) Install all required dependencies with the help of the `./trustdble` script (if you are using linux): `./trustdble init -s linux`

1) For Fabric:

- Clone the main branch of [fabric-samples](https://github.com/hyperledger/fabric-samples) by running `git clone https://github.com/hyperledger/fabric-samples.git` in a directory of your choice. The adapter needs features, that aren't included in any of the tagged releases, yet. It works with [commit 3d3f15ce301cca2e1e7847a5ae5d04aad42848d3](https://github.com/hyperledger/fabric-samples/commit/3d3f15ce301cca2e1e7847a5ae5d04aad42848d3). The features will probably be included in v2.4
- Download the binaries and docker images. This can be done by executing `curl -sSL https://raw.githubusercontent.com/hyperledger/fabric/main/scripts/install-fabric.sh | bash -s d b`

2) Edit paths in the config values (**only required if not running inside the devcontainer**):

- In ethereum/scripts/start-network.sh
    - dockerFilePath=${4:-`<your path>`/blockchain-manager/ethereum/scripts/ethereum-docker}
- In cli/src/config.ini
    - [Manager-Ethereum]
    - start-script-path=`<your path>`/blockchain-manager/ethereum/scripts/start-network.sh
    - docker-file-path=`<your path>`/blockchain-manager/ethereum/scripts/ethereum-docker
    - [Manager-Fabric]
    - start-script-path=`<your path>`/blockchain-manager/fabric/scripts/network.sh
    - test-network-dir=`<path to fabric-samples>`/test-network

3) To build this repo run:

```
./trustdble build
```

Alternatively use the following cmake commands to configure and perform the build using cmake

```
cmake -S . -B build-debug
cmake --build build-debug
```

## Run CLI locally
0) Change to the build folder containing the executable
```
cd build-debug/cli/src/
```
1) Please create a stub_data folder in the blockchain-manager folder for use stub-network-path.
2) Run the bc-manager cli tool
```
./managerCli
```
3) Follow the instructions on the screen, in particular: Specify 'ETHEREUM' or 'FABRIC' as a type and 'create' as a command. Lastly, type in a name for your network, e.g. 'network1'

## Run CLI inside a container for Ethereum and Stub
**Note**: The prerequisite is that the blockchain manager has access to the socket of the docker daemon running in the host. This is achieved by binding the docker.sock from the host in the container. For the devcontainer, we specified this in the `.devcontainer/devcontainer.json` in the `mounts` property.

If you want to build and use a custom docker container you can follow the following steps.

1) Build the docker image by running the following command from the workspace directory:
```
docker build -t bcmanager -f .devcontainer/Dockerfile .
```
(building the image might take a while since all the build dependencies have to be installed)
2) Run the container while binding the docker socket from the host inside the container:
```
docker run -it -v /var/run/docker.sock:/var/run/docker.sock -v `pwd`:/workspace bcmanager /bin/bash
```
3) You are now running inside the container, build the blockchain-manager tool:
```
cd workspace
rm -rf build
./trustdble build
```
4) Run the manager cli:
```
cd build-debug/cli/src
./managerCli
```
5) Follow the instructions on the screen, in particular: Specify 'ETHEREUM' as a type and 'create' as a command. Lastly, type in a name for your network, e.g. 'network1'

## Run tests
Edit paths in the config values:

- In ethereum/manager/tests/test-config.ini
    - start-script-path=`<your path>`/blockchain-manager/ethereum/scripts/start-network.sh
    - docker-file-path=`<your path>`/blockchain-manager/ethereum/scripts/ethereum-docker
- In stub/manager/tests/test-config.ini
    - stub-network-path=`<your path>`/blockchain-manager/stub_data/
- In blockchain-manager/ethereum/scripts/start-network.sh
    - dockerFilePath=${4:-`<your path>`/blockchain-manager/ethereum/scripts/ethereum-docker}
- In fabric/manager/tests/test-config.ini
    - start-script-path=`<your path>`/blockchain-manager/fabric/scripts/network.sh
    - test-network-dir=`<path to fabric-samples>`/test-network

Build this repo (debug mode by default):
```
./trustdble build
```
Run tests:
```
./trustdble test
```

## Git Hooks

This project uses pre-commit and pre-push scripts to ensure the quality of code.
The hooks are installed when you run the `trustdble init` helper script. Alternatively, use `git config core.hookspath .githooks` to install the hooks.

Alternatively, run the hook script directly, e.g., `.githooks/pre-push.d/01_sanitizer_check.sh` (Note some hook script might require you to stage files (`git add`) or use an additional flag).​

Use `git push/commit --no-verify` to skip the execution of hooks and only perform the corresponding git command (only do this if you know what are you doing).