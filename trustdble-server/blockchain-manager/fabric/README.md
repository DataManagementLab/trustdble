# Blockchain Manager Fabric
The Blockchain Manager Fabric can be used to automatically deploy a local fabric network using fabric's test-network
[fabric's test-network](https://github.com/hyperledger/fabric-samples/tree/main/test-network) and join peers to a local
test-network.

## Joining a network

The method `join_network(name, config)` creates a peer of the given name and joins it to the network. It currently expects the test-network's directory to be present and can thus only be used to join a local test-network.

## Leaving a network

The method `leave_network(name, config)` removes the peer of the given name from the network.

## About names

In a non test setup one machine joins a network only once and has only one peer per network. Here the peer can carry the name of the network.
When running a local test network, one machine runs multiple peers of the same network. Thus the peers' names can't identify their network, but are used to identify the peer itself.

## Creating a network

The method `create_network(name, config)` first starts the test-network consisting of two organisations with two peers each, an
orderer, a channel for the peers and a ca-server for every organisation and the orderer. The ca-servers aren't started by
default, but are needed to create certificates for joining peers.
A peer with the given name is joined to the network, because the interface is expected to support leaving the network with the name used to create the network. The initial two peers can't be used for this, because their names are hard-coded.

## Clean up

After leaving a network with the last peer, the initial two peers, the orderer and the ca-servers are stil present. All containers
joined to the network are given a label. When removing a peer, the block-chain manager checks if there are still containers with
this label. When there is no container with the label left, the initial network-components are removed, too.