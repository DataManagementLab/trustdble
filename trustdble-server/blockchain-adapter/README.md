# TrustDBle Adapters {#trustDBle_adapters}

This repo contains the definition of an adapter interface and various adapter implementation. The adapter interface defines how a TrustDBle can interact with a blockchain to store/retrieve data. There are currently implementations of this interface for Hyperledger Fabric, Ethereum and a stub blockchain that allows Trustdble to run without the need for a deployed blockchain. Details on the individual adapters can be found in corresponding subfolders.

## Project Structure

All (sub)folders containing C++ code follow the project structure describe in this [book](https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html) (please make sure to read it if you want to contribute).

```
├── .githooks/                      # Contains custom git hooks that we use for this repo, inspired by https://gist.github.com/damienrg/411f63a5120206bb887929f4830ad0d0
│   ├── pre-commit.d                # Scripts that are executed before a commit is preformed
│   │   ├── 01_static-analysis.sh   # Checks the code for coding best practices using clang-tidy
|   |   └── 02_code-formatting.sh   # Checks the formatting of the code before a commit
|   ├── pre-push.d                  # Scripts that are executed before code is pushed to the repository
│   │   ├── 01_doxygen.sh           # Generate the project documentation and report any missing documentation as errors
|   |   └── 02_sanitizer_check.sh   # Executes tests with address sanitizer enabled
|   ├── pre-commit                  # The actual hook, calls scripts in the corresponding folder
|   └── pre-push                    # The actual hook, calls scripts in the corresponding folder
├── .vscode/                        # Contains standard vscode settings, e.g., file associations
├── cli/                            # Contains a CLI application that can be to used to create an adapter to connect to a blockchain
├── ci/                             # Contains scripts and assets that are used for the CI pipeline
├── cmake/
|   ├── test_macros.cmake           # Macro to register new tests with gtest/ctest
|   └── trustdble_configure.cmake   # Custom cmake flag/variable definitions to e.g., compile with ASAN
├── docs/
│   ├── CMakeLists.txt              # Configuration of Doxygen, overview documentation
│   ├── doxygen-awesome.css         # Custom stylesheet to format doxygen HTML output
|   └── ...                         # Several Markdown files containing documentation
├── ethereum/                       # Assets to implement the Ethereum adapter
│   ├── adapter                     # C++ implementation of the adapter interface for ethereum
│   ├── contract                    # Assets for the deployment of the required blockchain contracts
|   └── scripts                     # Scripts to orchestrate the contract assets and prepare the blockchain for an interaction with the blockchain
├── fabric/                         # Assets to implement the Fabric adapter
│   ├── adapter                     # C++ implementation of the adapter interface for fabric
│   ├── contract                    # Assets for the deployment of the required blockchain contracts
|   └── scripts                     # Scripts to orchestrate the contract assets and prepare the blockchain for an interaction with the blockchain
├── factory/                        # C++ Factory module that helps to create the different types of adapters
├── interface/                      # C++ Definition of the generic adapter interface, contains also generic tests that should work for all implemented adapters
├── stub/                           # Assets to implement the Stub adapter
│   └── adapter                     # C++ implementation of the adapter interface for stub
├── trustdble_scripts/              # Collection of helper scripts to work with the code in this repository (e.g., install dependencies), these are orchestrated by the trustdble script in this folder
|   ├── ...
├── .clang-format                   # Clang-format config file, we use the Google coding guidelines
├── .clang-tidy                     # Clang-tidy config file for static code analysis
├── .gitignore                      # Generic .gitignore file, the different adapters might define additional ignores in their subfolders
├── bitbucket-pipelines.yml         # Bitbuckt Pipelines config file that defines our CI process, uses scripts in the ci/ folder
├── CMakeLists.txt                  # Main CMake configuration file that includes the different subfolders/modules
├── README.md
├── test-ci.sh                      # Script used to test our CI process locally (without pushing to bitbucket)
└── trustdble                       # Main helper script that facilitates working with this repository, e.g. setting up the dev environment by installing dependencies, building the project etc.
```

## Getting Started

1. Clone this repo with

```
git clone git@bitbucket.org:trustdble/trustdble-adapters.git
```

2. Install the requirements using the `trustdble` helper script:

```
./trustdble init -s [linux|macos]
```

3. Build the code using the `trustdble` helper script:

```
# this builds all cmake targets (including tests and docu)
./trustdble build
```

Alternatively use the following cmake commands to configure and perform the build using cmake

```
cmake -S . -B build
# Build all targets
cmake --build build
# Build a specific adapter, e.g., fabric
cmake --build build --target adapterFabric
```

5. Run the code: Please look at the Readme in the subfolder of each adapter for instructions on how to setup the corresponding blockchain. After that you can run the tests or use the adapters to interact with the blockchain

## Requirements

All requirements can be installed by using the `init` command of the `trustdble` helper script

General requirements

-   C++ std-17
-   CMake build system

Adapter specific requirements (e.g., golang) are mentioned in the Readme of the corresponding subfolders.

### Adapter Implementations

Currently there exists an implementation for the following blockchains

-   [Stub](stub) - simulation of a blockchain, uses a simple hashmap and persists data to disk
-   [Fabric](fabric) - Adapter implementation for the [Hyperledger Fabric](https://www.hyperledger.org/use/fabric) blockchain
-   [Ethereum](ethereum) - Adapter implementation for the [Ethereum](https://ethereum.org/) blockchain

## CI Pipeline

All artifacts required for the `ci-pipeline` are located in the `ci` directory.
The scripts in that folder contain the logic that will be executed in the pipeline.
Please note that our testing step currently only uses the stub adapter, since it does not need any running blockchain.

To build the docker image run

    docker build -t ci-adapters -f ci/Dockerfile .

Note: The Dockerfile uses the `init.sh` helper script to install dependencies, hence, we have to pass in the current folder (`.`) as build context.

Now you can test the `ci-pipeline` locally by executing the docker container and manually executing the `ci-scripts`

    bash test-ci.sh # this will start a docker container and mount the repo as a volume
    bash ci/build.sh g++
    bash ci/build.sh clang++
    bash ci/unit-test.sh

### Push image to Docker Hub

Create local image

    docker build . -f ci/Dockerfile -t ci-adapters

First login to the TrustDBle Docker Hub Account

    docker login --username trustdble --password '<secret-password>'

To push the docker image to Docker Hub, tag the image correctly

    docker tag ci-adapters:latest trustdble/ci:adapters

and then push it to Docker Hub

    docker push trustdble/ci:adapters

## Git Hooks

This project uses pre-commit and pre-push scripts to ensure the quality of code.
See the description under [Project Structure](#Project-Structure) for a short explanation of each hook.
The hooks are installed when you run the `trustdble init` helper script. Alternatively, use `git config core.hookspath .githooks` to install the hooks.

Use `git push/commit --dry-run` to run the hooks without actually executing the git command. Alternatively, run the hook script directly, e.g., `.githooks/pre-push.d/01_doxygen.sh` (Note some hook script might require you to stage files (`git add`) or use an additional flag).​

Use `git push/commit --no-verify` to skip the execution of hooks and only perform the corresponding git command (only do this if you know what are you doing).
