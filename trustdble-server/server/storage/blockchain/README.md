# Blockchain Storage Engine

This storage engine can be used to store tables on different blockchains. Therefore an adapter interface has to be implemented for each Blockchain-Technology.  
Currently there is an Ethereum adapter, a Fabric adapter and a simple Stub adapter to test the storage engine without deploying a hole blockchain network.

***
# Setup

1. Clone this repository
2. Install [boost version 1.73.0](https://www.boost.org/users/history/version_1_73_0.html) on your system.

***
# Build mysql-server and blockchain storage engine

## Prerequisites

- Check if _cmake_ is on your system:  
``` cmake --version ``` 

    - If not install it:  
    ```
    sudo apt install cmake
    ```


- Check if _ninja_ is on your system:  
``` ninja --version ```

    - If not install it:  
    ```
    sudo apt install ninja
    ```

## Build (with build.sh)

1.Run build.sh script with:  
    ```
    ./build.sh
    ```

## Build (single steps)
1. Create build folder:  
    ```
    mkdir -p build
    ```

2. In build folder create build files:  
    ```
    cd build 
    ```  
    ```
    cmake -DCMAKE_BUILD_TYPE=Debug -GNinja -DWITH_UNIT_TESTS=OFF -DCMAKE_CXX_STANDARD_LIBRARIES="-lcurl" ..
    ```

3. Compile build files  
    ```
    ninja
    ```
4. From root folder create plugin folder in build/lib  
    ```
    cd ..
    ```  
    ```
    mkdir -p build/lib/plugin
    ```
5. Copy compiled blockchain storage engine to plugin folder  
    ```
    cp build/plugin_output_directory/ha_blockchain.so build/lib/plugin
    ```

## Initial setup

Only has to be done once  
    ```
    cd trustdble-server
    ```  
    ```
    mkdir test_data_dir
    ```  
    ```
    build/bin/mysqld --datadir=$(pwd)/test_data_dir --basedir=$(pwd)/build --initialize-insecure --user=$(whoami)
    ```

## Run mysqld with blockchain plugin
Go to root folder:  
    ```
    cd trustdble-server
    ```  
    
* Add instructions for starting with Fabric here

* Start server with Ethereum Adapter:  
    ```
    build/bin/mysqld --binlog-format=STATEMENT --datadir=$(pwd)/test_data_dir --basedir=$(pwd)/build --plugin-load=ha_blockchain.so --blockchain-bc-type=0 --blockchain-bc-connection='http://localhost:8545' --blockchain-bc-eth-contracts=tableName:contractAddress,... --blockchain-bc-eth-from='accountFromAddress'
    ```  
* Start server with Stub Adapter:  
    ```
    mkdir stub_data
    ```  
    ```
    export MYSQL_HOME=$(pwd)/mysql_home1
    ```  
    ```
    build/bin/mysqld
    ```

## Configure trustdble-server

Edit my.cnf in mysql_home1

The following parameters can be used

| Parameter name            | Value Type | Value range | Functionality                                                                                                          |
|---------------------------|------------|-------------|------------------------------------------------------------------------------------------------------------------------|
| bc_type                   | INT        | 0 , 1       | Which Blockchain to use. Currently only Ethereum, Fabric and Stub are supported                                                |
| bc_connection             | String     |             | The HTTP URL to connect to a Blockchain node                                                                           |
| bc_use_ts_cache           | INT        | 0-1         | Configure whether the table scan cache should be used or not                                                           |
| bc_tx_prepare_immediately | INT        | 0-1         | Configure whether write operations should be send immediately to the Blockchain or only send in a batch on commit      |
| bc_eth_contracts          | String     |             | Comma-separated string to configure contract instance addresses for tables                                             |
| bc_eth_tx_contract        | String     |             | Address of transaction contract                                                                                        |
| bc_eth_from               | String     |             | Account address to use for transactions                                                                                |
| bc_eth_max_waiting_time   | INT        | 16-300      | Maximum time to wait until a transaction is mined, i.e. included in a block (in seconds)                               |
| bc_stub_path              | String     |             | Path to data folder of stub adatper. Can be used to share tables over multiple servers by setting all to the same path |


## MySQL client
* Use client to interact with server:  
    ```
    cd trustdble-server
    ```  
    ```
    export MYSQL_HOME=$(pwd)/mysql_home1
    ```  
    ```
    build/bin/mysql
    ```  
* Create new database and use it:  
    ```
    CREATE DATABASE test_db;
    ```  
    ```
    USE test_db;
    ```  
* Create table with blockchain storage engine:  
    ```
    CREATE TABLE test (id VARCHAR(4), name VARCHAR(10), mail VARCHAR(30)) ENGINE=BLOCKCHAIN;
    ```  
* Add row to table and show hole table:  
    ```
    INSERT INTO test VALUES (1, "Test-Name", "mail@test.de");
    ```  
    ```
    SELECT * FROM test;
    ```

## Stop mysqld
* Stop server:  
    ```
    sudo killall mysqld
    ```

## Start another server and access same table
Open new terminal and go to trustdble-server

* Stop running server
    ```
    sudo killall mysqld
    ```
* Copy data folder of mysql
    ```
    cp test_data_dir test_data_dir2
    ```
* Export second MYSQL_HOME with second my.cnf file
    ```
    export MYSQL_HOME=$(pwd)/mysql_home2
    ```
* Start first server as above in another terminal
* Start second server
    ```
    build/bin/mysqld
    ```
## Start client for second server
Open new terminal and go to trustdble-server

* Export second MYSQL_HOME
    ```
    export MYSQL_HOME=$(pwd)mysql_home2
    ```
* Start client
    ```
    build/bin/mysql
    ```
