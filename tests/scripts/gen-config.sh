#!/bin/bash

# generate a configuration for each node
cat <<EOT >> settings.yaml
# Gossip interval
interval: 1000000

# Port
port: 85$(printf "%02d" $1)

# Path to the nodes' private key
keyPath: "../tests/config/node$1/server.key"

# Path to the nodes' certificate
certPath: "../tests/config/node$1/server.cert"

# Path to the database file
databasePath: "node$1.sqlite"

# List of all nodes in the hashgraph network
network: 
EOT

# list of network nodes
for (( j=0; j<=$2; j++ )) do
cat <<EOT >> settings.yaml
  - 
    host: "localhost"
    port: 85$(printf "%02d" $j)
    certPath: "../tests/config/node$j/server.cert"
EOT
done