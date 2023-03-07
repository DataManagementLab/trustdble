#!/bin/bash

# This script is used to generate a private key for an ethereum account. If the private key exists, it is not overwritten.
# It is called inside the container before starting the ethereum node. The container mounts a volume to the resoucre folder
# in which the key is stored. With that the key will be used for all ethereum nodes by mounting the volume.

# Create ethereum resource folder
mkdir -p resources

cd resources

# check whether the key exists, if so exit
if [ -f privateKey ]; then
    exit 0
fi

# Install openssl
apt-get update
apt-get install -y openssl

# Generate private key
openssl ecparam -name prime256v1 -genkey -noout -out privkey.pem
# Extract public key
openssl ec -in privkey.pem -pubout -out pubkey.pem
# Save private key in hex
openssl ec -in privkey.pem -text -noout | awk '/priv:/{getline; print; getline; print; getline; print;}' | tr -d ' ' | tr -d '\n' | tr -d ':' > privateKey