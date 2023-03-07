# Pull base image
FROM ubuntu:20.04
# Define variables

# Defines geth version to use, must include commit info as shown at https://geth.ethereum.org/downloads/
# Resulting command is e.g., RUN wget https://gethstore.blob.core.windows.net/builds/geth-linux-amd64-${GETH_VERSION}.tar.gz
ARG GETH_VERSION=1.10.26-e5eb32ac

# Define working directory
WORKDIR /root

# Copy shell scripts to container (for installation and start)
ADD *.sh /root/

# invoke script to install specific geth
RUN /bin/bash installSpecificGeth.sh ${GETH_VERSION}

CMD bash ./start-ethereum.sh
