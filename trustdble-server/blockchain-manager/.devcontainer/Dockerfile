# See here for image contents: https://github.com/microsoft/vscode-dev-containers/tree/v0.155.1/containers/ubuntu/.devcontainer/base.Dockerfile

# [Choice] Ubuntu version: bionic, focal
ARG VARIANT="focal"
FROM mcr.microsoft.com/vscode/devcontainers/base:0-${VARIANT}

# copy init script from repo
# for this to work you have to specify the correct relative build context and pass the Dockerfile location via a flag
# (see devcontainer.json or in terminal from the root dir of the repo do `docker build -t trustdble-server -f .devcontainer/Dockerfile`)
COPY trustdble_scripts/init.sh init.sh
# execute the init script inside the container to install all dependencies
RUN apt update -y && apt -y install lsb-release wget software-properties-common
RUN /bin/bash init.sh -s linux
