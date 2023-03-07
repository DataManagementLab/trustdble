#!/bin/bash

# INSTALL PREREQUISITES

echo "[INIT] Install Adapter Prerequisites"

if [ $SYSTEM == "linux" ] ; then
    sudo apt-get -y install openjdk-11-jdk

    # Prerequisites to install latest version of go (REF: https://github.com/golang/go/wiki/Ubuntu)
    sudo apt-get -y install software-properties-common gpg
    sudo add-apt-repository -y ppa:longsleep/golang-backports

    sudo apt-get -y install apt-transport-https ca-certificates curl gnupg lsb-release

    sudo apt-get -y update

    sudo apt-get -y install npm nodejs

    # On systems that did not interact with the github server
    # The below truffle install command will prompt to trust the github server key
    # To avoid this promt, we add the server key to known_hosts if required
    github_server_key_known=$(ssh-keygen -F github.com)
    if [ $? -ne 0 ]; then # check the return value of ssh-keygen command
        ssh-keyscan -t ECDSA github.com >> $HOME/.ssh/known_hosts
        echo "Added missing github server key to known_hosts"
    fi
    sudo npm install truffle -g

    # Installing go after apt update
    sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 52B59B1571A79DBC054901C0F6BC817356A3D45E
    sudo apt-get -y install golang-go

elif [ $SYSTEM == "macos" ] ; then
    brew install go openjdk@11
    brew install --cask docker
fi
