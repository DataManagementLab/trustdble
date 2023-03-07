#!/bin/bash

COMPILER=${1:-"g++"}
echo "Using compiler ${COMPILER}"

rm -rf build
cmake -S . -B build -DCMAKE_CXX_COMPILER=${COMPILER} #-DCMAKE_CXX_STANDARD_LIBRARIES="-lcurl"
cmake --build build