#!/bin/bash

# File containing global methods used across trustdble scripts.

# IMPORTANT: 
# Methods are imported in trustdble.sh in the root directory. 
# Make sure to use the 'export' statement for any function to make it available for trustdble_scripts.

install_plugins () # Argument: plugin-string
{
  echo "creating plugin folder in build: ${PLUGIN_FOLDER}"
  mkdir -p ${PLUGIN_FOLDER}
  # iterate over plugins
  echo "installing plugins"
  for i in $(echo $1 | sed "s/;/ /g")
  do
    # install plugin
    echo "  installing $i"
    cp ${PLUGIN_OUTPUT_FOLDER}/$i ${PLUGIN_FOLDER}
  done
}
export -f install_plugins

load_plugins () # Argument: config-file
{ 
  PLUGINS=$(awk '/^plugin-load/ {split($0, a, "="); gsub(/^[ \t]+|[ \t]+$/, "", a[2]); print a[2]; exit}' $1)
}
export -f load_plugins