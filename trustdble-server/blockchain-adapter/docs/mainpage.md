TrustDBle Blockchain Adapters                            {#mainpage}
============

The TrustDBle Blockchain Adapters serve as an abstraction layer on top of blockchains to facilitate READ and WRITE operations from/to blockchains.
This abstraction layer assumes a key-value store data model in which the data is managed. The most simple analogy is a classical hashmap. 
The actual definition of the _key_ or _value_ (of a hashmap) is done by the application using a TrustDBle adapter.
For example, in case of trustdble-server, we use the primary key of a record as a key and the value is a byte serialized record.

The interface support the creation of multiple _tables_, i.e., hashmaps in which data can be stored.
The actual methods that are defined by the interface can be found in the class \ref BcAdapter.

Currently, there exist adapter implementations for the following blockchains:
* \ref ethereum_adapter 
* \ref fabric_adapter 
* \ref stub_adapter

Using the adapters usually requires connecting to an operational network of the corresponding blockchain platform.
Each adapter defines specific configuration values that must be set to use the adapter (e.g., the network connection information).
Please refer to the adapters corresponding README file and documentation to learn more about how to use an adapter.