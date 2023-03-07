#!/bin/bash

docker run -it \
  --volume=$(pwd):/localRepository \
  --workdir="/localRepository" \
  --memory=2048m \
  ci-adapters:latest /bin/bash
