FROM ubuntu:20.04 AS build-stage

# suppress region selection during dependency installation
ARG DEBIAN_FRONTEND=noninteractive

# install dependencies and build tools
RUN apt-get update && apt-get install --no-install-recommends -qq -y \
    cmake \
    automake \
    bison \
    flex \
    g++ \
    git \
    libboost-all-dev \
    libevent-dev \
    libssl-dev \
    libtool \
    make \
    pkg-config \
    openssl

# copy the source code into the container
COPY . /hashgraph

# switch to build folder
WORKDIR /hashgraph/build

# build
RUN cmake .. && make


# everything within the export-stage will be exported to the host
FROM scratch AS export-stage

# copy executable from build-state to export-stage
COPY --from=build-stage /hashgraph/build/hashgraph /build/hashgraph

# copy crypto material from build-state to export-stage
COPY --from=build-stage /hashgraph/tests/config /tests/config