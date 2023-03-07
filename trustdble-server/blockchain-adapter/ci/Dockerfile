FROM ubuntu:20.04

# ENVIRONMENT
# Avoid interaction during apt-get installs
ENV DEBIAN_FRONTEND=noninteractive
# copy init script from repo
# for this to work you have to specify the correct relative build context and pass the Dockerfile location via a flag
# (see devcontainer.json or in terminal from the root dir of the repo do `docker build -t ci-adapters -f ci/Dockerfile .`)
COPY trustdble_scripts/init.sh init.sh
# Remove sudo from the script since it is not installed and needed
RUN sed -i 's/sudo //g' init.sh
# execute the init script inside the container to install all dependencies
RUN /bin/bash init.sh -s linux