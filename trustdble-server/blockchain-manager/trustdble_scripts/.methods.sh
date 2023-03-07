#!/bin/bash

# File containing global methods used across trustdble scripts.

# IMPORTANT: 
# Methods are imported in trustdble.sh in the root directory. 
# Make sure to use the 'export' statement for any function to make it available for trustdble_scripts.

command_exists ()
{
   command -v $1 &>/dev/null
   return $?
}

contains ()
{
   if [[ $1 =~ $2 ]] ; then
      return 0
   else
      return 1
   fi
}
