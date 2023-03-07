# Stub Adapter {#stub_adapter} 

The stub adapter is a simple simulation of a blockchain backend that stores data in text files.
Its purpose is to provide all functionality of a BCAdapter without the need to deploy a real blockchain network.

## Design Considerations
* Each table is mapped to a dedicated `.txt` file on disk
* The files are stored in the path set by the `Adapter.data-path` config value
* To support distributed setups (ie. server instances running on different physical nodes) use a path on file share as `data-path`.
* The stub adapter locks the file only on writes for the duration of a write.
* Data is encoded as hex and then written to the file.