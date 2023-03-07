#!/bin/bash

cmake --build build --target test -- ARGS="-R Stub* --output-on-failure"
