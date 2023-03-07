#!/bin/bash
cd ../adapter/extern/go_client
go build -buildmode c-shared -o libclient.so trustdbleClient.go