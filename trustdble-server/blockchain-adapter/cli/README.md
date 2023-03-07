
# TrustDBle Adapter CLI

The TrustDBle Adapter command line interface (CLI) provides an easy way to use and test our AdapterInterface to interact with tables on an existing blockchain.
In case of using Stub as type the blockchain is simulated by a simple data folder storing files representing tables.

## Getting started

1) Adjust the file paths in the required *config.ini files in this repository.

    *Note: Make sure when using STUB as type that the data-path in the config.ini in the cli/src folder is valid and exists.*

2) Build the code by following the instructions in the README file located in the root of the repository

3) Switch to the location of the adapterCli executable in the build directory

        cd build-debug/cli/src

4) Run the adapterCli executable (optional: provide a path to a config file)

        ./adapterCli

5) You will be prompted to give the following input:

    1. Blockchain Type: one of [ETHEREUM, STUB]

    2. Network Configuration: This is adapter specific and corresponds to a database (e.g., for STUB the database-name, have a look at the README file of the adapter for more information)

6) Follow the instructions of the CLI to interact with the adapter













