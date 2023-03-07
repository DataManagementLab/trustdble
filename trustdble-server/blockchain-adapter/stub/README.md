# TrustDBle Stub Adapter

This adapter is a simulation for an adapter that writes data to a blockchain.
Instead of writing to an actual blockchain, however, this adapter writes data to a hashmap that is persisted on disk.
The purpose of this implementation is to test the adapter interface without deploying a real blockchain.

## Getting started

1. Adjust the configuration values in `config.ini`

## Explanation of config values

`data-path` := defines the folder in which the adapter will create files to persist its data
