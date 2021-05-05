#!/bin/bash

BASE=$(dirname "$(readlink -f "$0")")

# prepare key folder
if [ -d "keys" ]; then
    rm -R keys
fi

# generate crypto material for hashgraph nodes
for (( i=0; i<=$1; i++ ))
do
    mkdir -p "$BASE/../keys/node$i"
    cd "$BASE/../keys/node$i"
    sh "../../scripts/gen-identity.sh"
done

# generate crypto material for users
for (( i=0; i<=$2; i++ ))
do
    mkdir -p "$BASE/../keys/user$i"
    cd "$BASE/../keys/user$i"
    sh "../../scripts/gen-identity.sh"
done