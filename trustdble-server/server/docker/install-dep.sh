#!/bin/bash

# CONSTANTS
export BOOST_VERSION="1.73.0"
export BOOST_VERSION_URL="1_73_0"
export BOOST_CHECKSUM="9995e192e68528793755692917f9eb6422f3052a53c5e13ba278a228af6c7acf"
export BOOST_DIR=/tmp/boost
export BOOST_INSTALL_PATH="/usr/local/include/boost/version.hpp"

apt update && export DEBIAN_FRONTEND=noninteractive
apt install -y wget xz-utils curl libnuma1 libaio1 libncurses5 g++ nodejs npm

# Install boost
mkdir -p "${BOOST_DIR}"
cd /tmp/boost
curl -L --retry 3 "https://boostorg.jfrog.io/artifactory/main/release/${BOOST_VERSION}/source/boost_${BOOST_VERSION_URL}.tar.gz" -o boost.tar.gz
echo "${BOOST_CHECKSUM}  boost.tar.gz" | sha256sum -c
tar -xzf boost.tar.gz --strip 1
./bootstrap.sh
./b2 install --without-python --prefix=/usr/local link=shared runtime-link=shared install
rm -rf ${BOOST_DIR}
rm -rf /var/cache/*
# move boost lib to /usr/lib/x86_64-linux-gnu/ so mysqld can find it when loading plugin
mv /usr/local/lib/* /usr/lib/x86_64-linux-gnu/

# Install docker
apt-get update
apt-get install -y apt-transport-https ca-certificates gnupg lsb-release
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | gpg --dearmor -o /usr/share/keyrings/docker-archive-keyring.gpg
echo "deb [arch=amd64 signed-by=/usr/share/keyrings/docker-archive-keyring.gpg] https://download.docker.com/linux/ubuntu $(lsb_release -cs) stable" | tee /etc/apt/sources.list.d/docker.list > /dev/null
apt-get update
apt-get install -y docker-ce docker-ce-cli containerd.io

apt autoremove

