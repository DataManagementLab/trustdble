#!/bin/bash

# DEFAULTS
PROGNAME=$(basename $0)
VARIANT=$(lsb_release -sr)
TAG="trustdble-server"
CLEAN=false

# USAGE FUNCTION
function usage()
{
   cat << HEREDOC
   This script can be used to create the trustdble server docker image.

   optional arguments:
     -h, --help                 show this help message and exit
     -v, --variant              ubuntu variant to use for build, default=latest (LTS)
     -c, --clean                remove previously created docker containers (both ethereum and trustdble server)
     -t, --tag                  used for naming the docker image
HEREDOC
}

# ARGUMENTS
while [[ $# -gt 0 ]]
do
key="$1"

case $key in
    -h|--help)
    usage
    exit 0
    ;;
    -v|--variant)
    VARIANT="$2"
    shift # past argument
    shift # past value
    ;;
    -t|--tag)
    TAG="$2"
    shift # past argument
    shift # past value
    ;;
    -c|--clean)
    CLEAN=true
    shift # past argument
    ;;
    *) shift ;;     # not used parameters
esac
done

if [ ${CLEAN} == true ] ; then
  bash trustdble_scripts/docker_clean.sh
fi

# Build with release flag each time, when docker build is executed
echo "Build trustdble with release flag"
bash trustdble build --release

# Copy build-release to docker build folder
rm -rf docker/resources/*
mkdir -p docker/resources
cp -u -r build-release docker/resources/

echo "Create trustdble-server image with variant ${VARIANT}..."
docker build --build-arg VARIANT=$VARIANT docker -t ${TAG} 