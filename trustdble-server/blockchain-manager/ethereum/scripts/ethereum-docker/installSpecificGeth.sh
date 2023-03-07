#!/bin/bash
apt-get update \
  && apt-get install -y wget \
  && rm -rf /var/lib/apt/lists/*

#trim qoutes that might be passed from Docker
temp="${1%\"}"
GETH_VERSION="${temp#\"}"
echo "Downloading geth version" ${GETH_VERSION}

wget -q "https://gethstore.blob.core.windows.net/builds/geth-linux-amd64-${GETH_VERSION}.tar.gz"
tar xvf "geth-linux-amd64-${GETH_VERSION}.tar.gz"
cp "geth-linux-amd64-${GETH_VERSION}"/geth /usr/bin/geth

#apt-get remove -y wget