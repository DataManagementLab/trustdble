# Ethereum Adapter {#ethereum_adapter}

The Ethereum Adapter essentially uses a hash table data structure within an Ethereum smart contract to store key-value data.
More information on the actual design of the smart contract can be found on this page \subpage ethereum_contract_design.

## Design Considerations
* Since there is no cpp Ethereum SDK for ethereum we use scripts written in Javascript to deploy a new instance of the smart contract to the blockchain.
Each smart contract corresponds to new (hash) _table_. This enables applications to, e.g., store records with different schemas in dedicated hash tables (contracts).
Note that the Ethereum account is currently hard coded in this script
* Due to the lack of an cpp SDK we use libcurl and the plain Ethereum [JSON-RPC API](https://ethereum.org/en/developers/docs/apis/json-rpc/) to interact with the blockchain
* The adapter has currently been tested using the geth client (Ethereum Go client)
