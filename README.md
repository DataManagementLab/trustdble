# TRUSTDBLE
A trusted data management system based on blockchains. It extends mysql-server with a additional storage engine to store data on a blockchain and a rewrite plugin that adds new 'shared' commands to create shared databases and shared tables. So multiple parties can collaborate on table at the same time. More in formation are inside the sub folders.

## Benchmark
This folder contains two Benchmarks that are used to test the performance of the trustdble-server.
## Dashboard
The dashboard consists of a backend that provides an API to interact with the server and a frontend that uses the API and provides an simple graphical user interface. An additional component is the blockchain monitor which can be used to audit changes to shared tables on a blockchain.
## Server
The main part of trustdble is a rewrite and storage plugin for mysql-server. These plugins and small additional components are inside this folder.