/*
Copyright 2020 IBM All Rights Reserved.

SPDX-License-Identifier: Apache-2.0
*/

package main

import "C"

import (
	"crypto/x509"
	"fmt"
	"github.com/hyperledger/fabric-gateway/pkg/client"
	"github.com/hyperledger/fabric-gateway/pkg/identity"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials"
	"io/fs"
	"io/ioutil"
	"log"
	"os"
	"path"
	"strings"
	"sync"
)

type conn struct {
	grpcConnection *grpc.ClientConn
	gatewayConnection *client.Gateway
	channel string
	contractName string
	users int
}

var connections map[string]*conn = make(map[string]*conn)
var connectionsMutex sync.RWMutex

// newGrpcConnection create a new gRPC connection to the Gateway server.
func newGrpcConnection(tlsCertPath, peerEndpoint, gatewayPeer string) (*grpc.ClientConn, error) {

	certificate, err := loadCertificate(tlsCertPath)
	if err != nil {
		return nil, fmt.Errorf("failed to load certificate: %w", err)
	}

	certPool := x509.NewCertPool()
	certPool.AddCert(certificate)
	transportCredentials := credentials.NewClientTLSFromCert(certPool, gatewayPeer)

	connection, err := grpc.Dial(peerEndpoint, grpc.WithTransportCredentials(transportCredentials))
	if err != nil {
		return nil, fmt.Errorf("failed to create gRPC connection: %w", err)
	}

	return connection, nil
}

// newIdentity creates a client identity for this Gateway connection using an X.509 certificate.
func newIdentity(mspID, certPath string) (*identity.X509Identity, error) {
	certificate, err := loadCertificate(certPath)
	if err != nil {
		return nil, fmt.Errorf("failed to load certificate: %w", err)
	}

	id, err := identity.NewX509Identity(mspID, certificate)
	if err != nil {
		return nil, fmt.Errorf("failed to create X509 identity: %w", err)
	}

	return id, nil
}

func loadCertificate(filename string) (*x509.Certificate, error) {
	certificatePEM, err := readFile(filename)
	if err != nil {
		return nil, fmt.Errorf("failed to read certificate file: %w", err)
	}
	return identity.CertificateFromPEM(certificatePEM)
}

// newSign creates a function that generates a digital signature from a message digest using a private key.
func newSign(keyPath string) (identity.Sign, error) {
	files, err := readDir(keyPath)
	if err != nil {
		return nil, fmt.Errorf("failed to read private key directory: %w", err)
	}
	privateKeyPEM, err := readFile(path.Join(keyPath, files[0].Name()))

	if err != nil {
		return nil, fmt.Errorf("failed to read private key file: %w", err)
	}

	privateKey, err := identity.PrivateKeyFromPEM(privateKeyPEM)
	if err != nil {
		return nil, fmt.Errorf("failed to get private key from file: %w", err)
	}

	sign, err := identity.NewPrivateKeySign(privateKey)
	if err != nil {
		return nil, fmt.Errorf("failed to get sign function: %w", err)
	}

	return sign, nil
}

func readFile(filename string) ([]byte, error) {
	homeDir, err := os.UserHomeDir()
	if err != nil {
		return nil, fmt.Errorf("failed to get home dir: %w", err)
	}
	absoluteFilename := strings.ReplaceAll(filename, "~", homeDir)
	return ioutil.ReadFile(absoluteFilename)
}

func readDir(dirname string) ([]fs.FileInfo, error) {
	homeDir, err := os.UserHomeDir()
	if err != nil {
		return nil, fmt.Errorf("failed to get home dir: %w", err)
	}
	absoluteDirname := strings.ReplaceAll(dirname, "~", homeDir)
	return ioutil.ReadDir(absoluteDirname)
}

//export Init
func Init(channel, contractName, mspID, certPath, keyPath, tlsCertPath, peerEndpoint, gatewayPeer string) int {
	log.Println("============ initializing go client ============")

	connectionsMutex.Lock()
	defer connectionsMutex.Unlock()
	connection, ok := connections[gatewayPeer]
	if !ok {
		log.Println("============ building grpc connection ============")
		clientConnection, err := newGrpcConnection(tlsCertPath, peerEndpoint, gatewayPeer)
		if err != nil {
			log.Printf("Failed to establish grpc connection: %v", err)
			return 1
		}

		id, err := newIdentity(mspID, certPath)
		if err != nil {
			log.Printf("Failed to create new client identiy: %v", err)
			return 1
		}

		sign, err := newSign(keyPath)
		if err != nil {
			log.Printf("Failed to create function for signing with clients private key: %v", err)
			return 1
		}

		// Create a Gateway connection for a specific client identity
		gatewayConnection, err := client.Connect(
			id,
			client.WithSign(sign),
			client.WithClientConnection(clientConnection),
		)
		if err != nil {
			log.Printf("Failed to connect to gateway: %v", err)
			return 1
		}
		connection = &conn{clientConnection, gatewayConnection, channel, contractName, 1}
		connections[gatewayPeer] = connection

		return 0
	}

	connection.users += 1

	return 0
}

//export Close
func Close(contractName, gatewayPeer string) int {

	log.Println("============ closing go client ============")
	connectionsMutex.Lock()
	defer connectionsMutex.Unlock()
	connection, ok := connections[gatewayPeer]
	if !ok {
		log.Printf("The client hasn't been initialized for gateway peer %v", gatewayPeer)
		return 1
	}

	connection.users -= 1

	if connection.users == 0 {
		log.Println("============ closing grpc connection ============")
		connection.gatewayConnection.Close()
		connection.grpcConnection.Close()
		delete(connections, gatewayPeer)
	}

	return 0
}

//export Write
func Write(json_value, function, tableName, gatewayPeer string) int {

	log.Println("============ " + function + " ============")

	connectionsMutex.RLock()
	defer connectionsMutex.RUnlock()
	connection, ok := connections[gatewayPeer]
	if !ok {
		log.Printf("The client hasn't been initialized for gateway peer %v", gatewayPeer)
		return 1
	}

	network := connection.gatewayConnection.GetNetwork(connection.channel)
	contract := network.GetContract(connection.contractName)

	_, err := contract.SubmitTransaction(function, tableName, json_value)
	if err != nil {
		log.Printf("Failed to submit transaction: %v", err)
		return 1
	}

	return 0
}

//export Read
func Read(json_value, function string, tableName, gatewayPeer string) (*C.char, int, int) {

	log.Println("============ " + function + " ============")

	connectionsMutex.RLock()
	defer connectionsMutex.RUnlock()
	connection, ok := connections[gatewayPeer]
	if !ok {
		log.Printf("The client hasn't been initialized for gateway peer %v", gatewayPeer)
		return nil, 0, 1
	}

	network := connection.gatewayConnection.GetNetwork(connection.channel)
	contract := network.GetContract(connection.contractName)

	value, err := contract.EvaluateTransaction(function, tableName, json_value)
	if err != nil {
		log.Printf("Failed to evaluate transaction: %v", err)
		return nil, 0, 1
	}
	return C.CString(string(value)), len(value), 0
}

// empty main is needed
func main() {}
