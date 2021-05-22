
# Hashgraph

Simple implementation of the consensus algorithm Hashgraph.

## How to build

```bash
# clone repo including submodules
git clone --recurse-submodules https://github.com/c3ai-lab/hashgraph
cd hashgraph/
 
# build everything
mkdir build
cd build/
cmake ..
make
```

## Build with Docker

```bash
DOCKER_BUILDKIT=1 docker build --output . .
```