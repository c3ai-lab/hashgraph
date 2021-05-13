#!/bin/bash

BASE=$(dirname "$(readlink -f "$0")")

# prepare key folder
if [ -d "config" ]; then
    rm -R config
fi

# generate all material required to run the network
for (( i=0; i<=$1; i++ ))
do
    mkdir -p "$BASE/../config/node$i"
    cd "$BASE/../config/node$i"
    bash "../../scripts/gen-crypto.sh"
    bash "../../scripts/gen-config.sh" $i $1
done
