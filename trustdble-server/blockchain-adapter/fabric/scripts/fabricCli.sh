#!/bin/bash

g++ -I ../adapter/include/ -I ../../interface/include/ ../adapter/apps/cli_fabric.cpp ../adapter/src/adapter_fabric.cpp ../adapter/src/client_fabric.cpp -pthread -o fabricCli ./../adapter/extern/go_client/libclient.so

#./fabric_cli